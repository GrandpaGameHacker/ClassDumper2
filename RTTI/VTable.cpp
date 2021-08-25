#include "VTable.h"

bool IsValid(void* VTable_start, SectionInfo* sectionInfo)
{
	auto* vtable = static_cast<uintptr_t*>(VTable_start);
	auto* meta = vtable - 1;
	if (sectionInfo->RDATA.base <= *meta && *meta <= sectionInfo->RDATA.end)
	{
		if (sectionInfo->TEXT.base <= *vtable && *vtable <= sectionInfo->TEXT.end)
		{
			auto* COL = reinterpret_cast<RTTI::CompleteObjectLocator*>(*meta);
#ifdef _WIN64
			if (COL->signature == 1)
			{
				auto* descriptor = COL->GetTypeDescriptor();
#else
			if (COL->signature == 0) {
				auto descriptor = COL->pTypeDescriptor;
#endif
				if (IsBadReadPointer(reinterpret_cast<void*>(descriptor)) != 0)
				{
					return false;
				}
				// Test if string is ".?AV" real quick
				if (*reinterpret_cast<unsigned long*>(&descriptor->name) == TYPEDESCRIPTOR_SIGNITURE)
				{
					return true;
				}
			}
			}
		}
	return false;
	}

bool IsValidSlow(void* VTable_start, SectionInfo * sectionInfo)
{
	{
		auto* vtable = static_cast<uintptr_t*>(VTable_start);
		auto* meta = vtable - 1;
		if (IsBadReadPointer(reinterpret_cast<void*>(vtable)))return false;
		if (IsBadReadPointer(reinterpret_cast<void*>(meta)))return false;
		if (sectionInfo->RDATA.base <= *meta && *meta <= sectionInfo->RDATA.end)
		{
			if (sectionInfo->TEXT.base <= *vtable && *vtable <= sectionInfo->TEXT.end)
			{
				auto* COL = reinterpret_cast<RTTI::CompleteObjectLocator*>(*meta);
#ifdef _WIN64
				if (COL->signature == 1)
				{
					auto* descriptor = COL->GetTypeDescriptor();
#else
				if (COL->signature == 0) {
					auto descriptor = COL->pTypeDescriptor;
#endif
					if (IsBadReadPointer(reinterpret_cast<void*>(descriptor)) != 0)
					{
						return false;
					}
					// Test if string is ".?AV" real quick
					if (*reinterpret_cast<unsigned long*>(&descriptor->name) == TYPEDESCRIPTOR_SIGNITURE)
					{
						return true;
					}
				}
				}
			}
		return false;
		}
	}

std::vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo * sectionInfo)
{
	std::vector<uintptr_t> functionList;
	uintptr_t vtable_decrypted = reinterpret_cast<uintptr_t>(VTable_start) ^ 0xDEADBEEF;
	auto* vtable = reinterpret_cast<uintptr_t*>(vtable_decrypted);
	auto function_ptr = *vtable;
	while (sectionInfo->TEXT.base <= function_ptr && function_ptr <= sectionInfo->TEXT.end)
	{
		functionList.push_back(function_ptr);
		vtable++;
		function_ptr = *vtable;
	}
	return functionList;
}

std::vector<uintptr_t> FindAllVTables(SectionInfo * sectionInfo)
{
	std::vector<uintptr_t> vtableList;
	uintptr_t ptr = sectionInfo->RDATA.base + sizeof(uintptr_t);
	while (ptr < sectionInfo->RDATA.end)
	{
		if (IsValid(reinterpret_cast<void*>(ptr), sectionInfo))
		{
			vtableList.push_back(ptr);
		}
		ptr += sizeof(uintptr_t);
	}
	return vtableList;
}

