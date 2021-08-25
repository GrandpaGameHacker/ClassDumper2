#include "Symbols.h"
const std::string VTABLE_SYMBOL_PREFIX = "??_7";
const std::string VTABLE_SYMBOL_SUFFIX = "6B@";
static char buff[0x1000];

std::string Symbols::DemangleMicrosoft(char* symbol)
{
	memset(buff, 0, 0x1000);
	char* pSymbol = symbol;
	// don't question this magic, it works :)
	if (*static_cast<char*>(symbol + 4) == '?') pSymbol = symbol + 1;
	else if (*static_cast<char*>(symbol) == '.') pSymbol = symbol + 4;
	else if (*static_cast<char*>(symbol) == '?') pSymbol = symbol + 2;
	else
	{
		//report error
	}
	std::string ModifiedSymbol = pSymbol;
	ModifiedSymbol.insert(0, VTABLE_SYMBOL_PREFIX);
	ModifiedSymbol.insert(ModifiedSymbol.size(), VTABLE_SYMBOL_SUFFIX);
	if (!((UnDecorateSymbolName(ModifiedSymbol.c_str(), buff, 0x1000, 0)) != 0))
	{
		//report error
		return std::string(symbol); //Failsafe
	}
	return std::string(buff);
}

void Symbols::StringFilter(std::string& string, const std::string& substring)
{
	size_t pos;
	while ((pos = string.find(substring)) != std::string::npos)
	{
		string.erase(pos, substring.length());
	}
}

static std::vector<std::string> filters =
{
	"::`vftable'",
	"const ",
	"::`anonymous namespace'"
};

void Symbols::FilterSymbol(std::string& symbol)
{
	for (auto& filter : filters)
	{
		StringFilter(symbol, filter);
	}
}

bool Symbols::SymbolComparator(uintptr_t v1, uintptr_t v2)
{
	v1 = v1 - sizeof(uintptr_t);
	v2 = v2 - sizeof(uintptr_t);
	auto* pv1 = reinterpret_cast<uintptr_t*>(v1);
	auto* pv2 = reinterpret_cast<uintptr_t*>(v2);
	auto* col1 = reinterpret_cast<RTTI::CompleteObjectLocator*>(*pv1);
	auto* col2 = reinterpret_cast<RTTI::CompleteObjectLocator*>(*pv2);
	auto* td1 = col1->GetTypeDescriptor();
	auto* td2 = col2->GetTypeDescriptor();

	const auto symbol1 = DemangleMicrosoft(&td1->name);
	const auto symbol2 = DemangleMicrosoft(&td2->name);

	if (symbol1 == symbol2)
	{
		return (v1 < v2);
	}
	return (symbol1 < symbol2);
}

void Symbols::SortSymbols(std::vector<uintptr_t>& vtableList)
{
	sort(vtableList.begin(), vtableList.end(), SymbolComparator);
}
