#pragma once
#include <Windows.h>
#include <vector>
#include "RTTI.h"
#include "../W32/Memory.h"
#include "../W32/Modules.h"

const unsigned int TYPEDESCRIPTOR_SIGNITURE = 0x56413F2E;
bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
bool IsValidSlow(void* VTable_start, SectionInfo* sectionInfo);
std::vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
std::vector<uintptr_t> FindAllVTables(SectionInfo* sectionInfo);
