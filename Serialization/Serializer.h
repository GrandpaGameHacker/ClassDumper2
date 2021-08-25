#pragma once
#pragma once
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "tinyxml2.h"
#include "..\RTTI\ClassMetadata.h"

namespace Serializer
{
	const static char* exportFileExtensions[] {".txt\0", ".xml\0",};
	const static char* importFileExtensions[] {".xml\0",};
	enum class ExportType {
		TEXT,
		XML,
	};

	enum class ImportType {
		XML
	};

	std::string SaveDialog(ExportType type, HWND owner);
	std::string OpenDialog(ImportType type, HWND owner);

	bool ExportClasses(const std::string& fileName, std::vector<ClassMetadata*>& classes, ExportType type);
	bool ExportTEXT(std::vector<ClassMetadata*>& classes, const std::string& fileName);
	bool ExportXML(std::vector<ClassMetadata*>& classes, const std::string& fileName);

	bool ImportClasses(const std::string& fileName, ImportType type, std::vector<ClassMetadata*>& classes);
	bool ImportXML(const std::string& fileName, std::vector<ClassMetadata*>& classes);
}

