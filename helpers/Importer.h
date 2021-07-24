#pragma once
#include <Windows.h>
#include <fstream>
#include <iostream>

#include "tinyxml2.h"

#include "ClassMeta.h"

enum class ImportType {
	XML
};

const static char* importFileExtensions[]
{
	".xml\0",
};

std::string OpenDialog(ImportType type, HWND owner);

bool ImportClasses(const std::string &fileName, ImportType type, std::vector<ClassMeta*>& classes);

bool ImportXML(const std::string &fileName, std::vector<ClassMeta*>& classes);