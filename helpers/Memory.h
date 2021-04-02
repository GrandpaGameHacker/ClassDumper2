#pragma once
#include <Windows.h>
#include <vector>
#include <future>
#include <thread>
#include <mutex>
#include "ModuleUtils.h"

bool IsBadReadPointer(void* p);
std::vector<uintptr_t> FindAllInstances(uintptr_t VTable, SectionInfo* sectionInfo);
std::vector<uintptr_t> FindReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue);