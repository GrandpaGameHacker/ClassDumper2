#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <DbgHelp.h>
#pragma comment(lib,"dbghelp.lib")
#include <algorithm>

#include "RTTI.h"
namespace Symbols {
std::string DemangleMicrosoft(char* symbol);
void StringFilter(std::string& string, const std::string& substring);
void FilterSymbol(std::string& symbol);

bool SymbolComparator(uintptr_t v1, uintptr_t v2);
void SortSymbols(std::vector<uintptr_t>& vtableList);
}
