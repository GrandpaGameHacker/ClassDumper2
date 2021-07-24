#include "Importer.h"
namespace TXML = tinyxml2;


std::string OpenDialog(ImportType type, HWND owner)
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
	ofn.lpstrFilter = "XML File (*.xml)\0*.xml";
	if (GetOpenFileName(&ofn)) {
		auto fileString = std::string(szFile);
		return fileString;
	}
	return "";
}

bool ImportClasses(const std::string &fileName, ImportType type, std::vector<ClassMeta*> &classes)
{
	switch (type) {
	case ImportType::XML:
		return ImportXML(fileName, classes);
		break;
	}
	return false;
}

bool ImportXML(const std::string &fileName, std::vector<ClassMeta*> &classes)
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
