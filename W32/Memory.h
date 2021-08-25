#pragma once
#include <Windows.h>
#include <vector>
#include <future>
#include <thread>
#include <mutex>
#include "Modules.h"

bool IsBadReadPointer(void* p);
bool IsBadReadPointerAligned(void* p);
std::vector<uintptr_t> FindAllInstances(uintptr_t VTable);
std::vector<uintptr_t> FindReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue);
std::vector<uintptr_t> FindCodeReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue);