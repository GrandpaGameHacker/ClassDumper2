#include "Serializer.h"

namespace TXML = tinyxml2;


std::string Serializer::SaveDialog(ExportType type, HWND owner)
{
	OPENFILENAME ofn;
	char szFile[MAX_PATH];
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;

	const char* f_ext = exportFileExtensions[0];
	switch (type) {
	case ExportType::TEXT:
		ofn.lpstrFilter = "Text file (.txt)\0*.TXT\0";
		break;

	case ExportType::XML:
		ofn.lpstrFilter = "XML file (.xml)\0*.XML\0";
		f_ext = exportFileExtensions[1];
		break;
	}
	if (GetSaveFileName(&ofn)) {
		auto fileString = std::string(szFile);
		if (fileString.find('.') == std::string::npos) {
			fileString += f_ext;
		}
		return fileString;
	}
	return "";
}



std::string Serializer::OpenDialog(ImportType type, HWND owner)
{
	OPENFILENAME ofn;
	char szFile[MAX_PATH];
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	switch (type)
	{
	case ImportType::XML:
		ofn.lpstrFilter = "XML File (*.xml)\0*.xml";
		break;
	}

	if (GetOpenFileName(&ofn)) {
		auto fileString = std::string(szFile);
		return fileString;
	}
	return "";
}

bool Serializer::ExportClasses(std::string const& fileName, std::vector<ClassMetadata*>& classes, ExportType type)
{
	switch (type) {
	case ExportType::TEXT:
		return ExportTEXT(classes, fileName);
		break;

	case ExportType::XML:
		return ExportXML(classes, fileName);
		break;
	}
	return false;
}

bool Serializer::ExportTEXT(std::vector<ClassMetadata*>& classes, const std::string& fileName)
{
	std::ofstream out;
	out.open(fileName);
	if (!out) {
		return false;
	}
	for (auto c : classes) {

		if (c->bVirtualInheritance || c->bMultipleInheritance || c->bAmbigious)
		{
			std::string s_inhr = "\t ";
			if (c->bVirtualInheritance) s_inhr += "V";
			if (c->bMultipleInheritance) s_inhr += "M";
			if (c->bAmbigious) s_inhr += "A";
			s_inhr += " ";
			out << s_inhr;
		}
		else {
			out << "    ";
		}

		out << "0x" << std::hex << ((uintptr_t)c->VTable ^ 0xDEADBEEF) << "\t";
		if (c->bInterface)
		{
			out << "interface ";
			out << c->className << " -> " << c->interfaceName;
		}
		else if (c->bStruct)
		{
			out << "struct ";
			out << c->className;
		}

		else {
			out << "class ";
			out << c->className;
		}
		out << "\n";
		if (c->size > 0) {
			out << std::hex << "\tSize: 0x" << c->size << "\n";
		}
		if (c->numBaseClasses >= 2) {
			out << "\tBase Classes (" << std::dec << c->numBaseClasses << "):\n";
			unsigned int tabIndex = 0;
			unsigned int lastOffset = -1;

			for (unsigned int i = 0; i < c->baseClassNames.size(); i++) {
				if (lastOffset == c->GetBaseClass(i + 1)->where.mdisp) {
					tabIndex += 1;
				}
				else {
					lastOffset = c->GetBaseClass(i + 1)->where.mdisp;
					tabIndex = 0;
				}
				std::string formatString = "\t";
				for (unsigned int x = 0; x < tabIndex; x++) { formatString.append("\t"); }
				formatString.append(c->baseClassNames[i]);
				out << "\t" << formatString << "\n";
			}
		}
		out << "\tVirtual Function Table (" << std::dec << c->VirtualFunctions.size() << "):\n";
		for (unsigned int i = 0; i < c->VirtualFunctions.size(); i++) {
			out << "\t\t" << std::hex << i * sizeof(void*) << "\t";
			out << std::dec << i << "\t";
			out << "\t0x" << c->VirtualFunctions[i] << "\t" << c->VirtualFunctionNames[i] << "\n";
		}
		out << "\n\n";
	}
	out.close();
	return true;
}

bool Serializer::ExportXML(std::vector<ClassMetadata*>& classes, const std::string& fileName)
{
	TXML::XMLDocument xmlDoc;
	TXML::XMLElement* pRoot = xmlDoc.NewElement("ClassDumper2");
	xmlDoc.InsertFirstChild(pRoot);
	TXML::XMLElement* pClasses = xmlDoc.NewElement("Classes");
	pClasses->SetText("\n");
	pClasses->SetAttribute("nClasses", classes.size());
	pRoot->InsertEndChild(pClasses);
	for (auto c : classes) {
		TXML::XMLElement* xclass = xmlDoc.NewElement("Class");
		xclass->SetText(c->className.c_str());
		xclass->SetAttribute("VTable", (uintptr_t)c->VTable ^ 0xDEADBEEF);
		xclass->SetAttribute("size", c->size);
		xclass->SetAttribute("size_locked", c->size_locked);
		xclass->SetAttribute("bInterface", c->bInterface);
		xclass->SetAttribute("interfaceName", c->interfaceName.c_str());
		xclass->SetAttribute("bStruct", c->bStruct);
		xclass->SetAttribute("bVirtualInheritance", c->bVirtualInheritance);
		xclass->SetAttribute("bMultipleInheritance", c->bMultipleInheritance);
		xclass->SetAttribute("bAmbigious", c->bAmbigious);
		xclass->SetAttribute("nFunctions", c->VirtualFunctions.size());
		for (unsigned int i = 0; i < c->VirtualFunctions.size(); i++) {
			TXML::XMLElement* vf = xmlDoc.NewElement("VirtualFunction");
			vf->SetText(c->VirtualFunctionNames[i].c_str());
			vf->SetAttribute("address", c->VirtualFunctions[i]);
			xclass->InsertEndChild(vf);
		}
		pClasses->InsertEndChild(xclass);
	}
	FILE* exportFile;
	fopen_s(&exportFile, fileName.c_str(), "w+");
	if (exportFile) {
		xmlDoc.SaveFile(exportFile);
		fclose(exportFile);
		return true;
	}
	return false;
};

bool Serializer::ImportClasses(const std::string &fileName, ImportType type, std::vector<ClassMetadata*> &classes)
{
	switch (type) {
	case ImportType::XML:
		return ImportXML(fileName, classes);
		break;
	}
	return false;
}

bool Serializer::ImportXML(const std::string &fileName, std::vector<ClassMetadata*> &classes)
{
	TXML::XMLDocument xmlDoc;
	FILE* importFile;
	fopen_s(&importFile, fileName.c_str(), "rb");
	if (xmlDoc.LoadFile(importFile) == TXML::XML_SUCCESS) {
		TXML::XMLElement* pRoot = xmlDoc.FirstChildElement("ClassDumper2");
		TXML::XMLElement* pClasses = pRoot->FirstChildElement("Classes");
		TXML::XMLElement* pClass = pClasses->FirstChildElement("Class");
		size_t nClasses;
		pClasses->QueryAttribute("nClasses", &nClasses);
		for (size_t i = 0; i < nClasses; i++) {
			pClass->QueryAttribute("size", &classes[i]->size);
			pClass->QueryAttribute("size_locked", &classes[i]->size_locked);
			TXML::XMLElement* vf = pClass->FirstChildElement("VirtualFunction");
			for (size_t q = 0; q < classes[i]->VirtualFunctions.size(); q++) {
				classes[i]->VirtualFunctionNames[q] = vf->GetText();
				vf = vf->NextSiblingElement();
			}
			pClass = pClass->NextSiblingElement();

		}
	}
	if (importFile) {
		fclose(importFile);
	}
	return false;
}
