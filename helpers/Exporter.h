#pragma once
#include <Windows.h>
#include <fstream>
#include <iostream>

#include "tinyxml2.h"

#include "ClassMeta.h"

enum class ExportType {
	TEXT,
	XML,
};

const static char * exportFileExtensions[]
{
	".txt\0",
	".xml\0",
};

std::string SaveDialog(ExportType type, HWND owner);

bool ExportClasses(const std::string &fileName, std::vector<ClassMeta*> &classes, ExportType type);

bool ExportTEXT(std::vector<ClassMeta*> &classes, const std::string &fileName);
bool ExportXML(std::vector<ClassMeta*> &classes, const std::string &fileName);
