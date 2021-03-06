#pragma once
#include <Windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include <TlHelp32.h>

struct Section
{
	uintptr_t base;
	uintptr_t end;
	size_t size;
};

struct SectionInfo
{
	uintptr_t ModuleBase;
	Section TEXT;
	Section RDATA;
};

bool IsSystemModule(MODULEENTRY32* Module);
std::vector<MODULEENTRY32*> GetModuleList(HMODULE skipModule);
SectionInfo* GetSectionInformation(MODULEENTRY32* Module);
uintptr_t GetRVA(uintptr_t VA, SectionInfo* sectionInfo);
