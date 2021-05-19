#include "Memory.h"
const DWORD RWEMask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
	PAGE_EXECUTE_WRITECOPY);

const DWORD RWMask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY);
const DWORD BadPageMask = (PAGE_GUARD | PAGE_NOACCESS);

bool IsBadReadPointer(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = {nullptr};
	if (VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		bool b = !(mbi.Protect & RWEMask);
		if (mbi.Protect & BadPageMask) b = true;

		return b;
	}
	return true;
}

// multithreaded memory scanner
// slightly modified to reduce self references due to being in the same memory space
std::vector<uintptr_t> FindAllInstances(uintptr_t VTable)
{
	//Customize to your liking how many threads
	//keep in mind in release versions you'll get self references more often due to optimizations
	const unsigned int threads = 12;
	std::vector<uintptr_t> instances;

	if (!VTable) {
		return instances;
	}
	// get page info
	SYSTEM_INFO sysinfo;
	MEMORY_BASIC_INFORMATION mbi;
	::GetSystemInfo(&sysinfo);
	uintptr_t start = (uintptr_t)sysinfo.lpMinimumApplicationAddress;
	uintptr_t max = (uintptr_t)sysinfo.lpMaximumApplicationAddress;
	size_t pageSize = sysinfo.dwPageSize;

	// get all valid regions
	std::vector<MEMORY_BASIC_INFORMATION*> mbiList;
	MEMORY_BASIC_INFORMATION* lastmbi = nullptr;
	for (uintptr_t current = start; current < max; current += mbi.RegionSize)
	{
		SIZE_T query = VirtualQuery((void*)current, &mbi, sizeof(mbi));
		if (!query) break;
		if (mbi.State != MEM_COMMIT || mbi.Protect & BadPageMask) continue;
		if (mbi.Protect == PAGE_READONLY || mbi.Protect == PAGE_EXECUTE_READ) continue;
		uintptr_t address = (uintptr_t)mbi.BaseAddress;
		auto p_mbi = new MEMORY_BASIC_INFORMATION();
		memcpy(p_mbi, &mbi, sizeof(mbi));
		mbiList.push_back(p_mbi);
	}
	bool notDone = true;
	unsigned int mbiIndex = 0;
	size_t mbiMax = mbiList.size();
	std::vector<uintptr_t> results[threads];
	std::future<std::vector<uintptr_t>> futures[threads];
	// run scans
	while (notDone) {
		if (mbiIndex == mbiMax) notDone = false;
		for (unsigned int i = 0; i < threads; i++) {
			if (mbiIndex < mbiMax) {
				MEMORY_BASIC_INFORMATION* currentMbi = mbiList[mbiIndex];
				mbiIndex++;
				if (currentMbi) {
					futures[i] = std::async(FindReferences, (intptr_t)currentMbi->BaseAddress, currentMbi->RegionSize, VTable);
				}
			}
		}
		for (unsigned int i = 0; i < threads; i++) {
			if (futures[i].valid()) {
				auto temp = futures[i].get();
				instances.insert(instances.end(), temp.begin(), temp.end());
			}
		}
	}

	mbiList.clear();
	return instances;
}

// modified to reduce self references
std::vector<uintptr_t> FindReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue)
{
	std::vector<uintptr_t> resultsList;
	uintptr_t newmem = (uintptr_t) malloc(length);
	if (!newmem) return resultsList;
	SIZE_T bytesRead = 0;
	if (!ReadProcessMemory((HANDLE)-1, (void*)startAddress, (void*)newmem, length, &bytesRead))
	{
		free((void*)newmem);
		return resultsList;
	}

	for (uintptr_t i = newmem; i < newmem + length; i += sizeof(uintptr_t))
	{
		uintptr_t candidate = *(uintptr_t*) i;
		if (candidate == (scanValue ^ 0xDEADBEEF))
		{
			uintptr_t realAddress = (i - newmem) + startAddress;
			resultsList.push_back(realAddress);
		}
	}
	free((void*)newmem);
	return resultsList;
}

#ifdef _WIN64
std::vector<uintptr_t> FindCodeReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue)
{
	std::vector<uintptr_t> resultsList;
	uintptr_t newmem = (uintptr_t)malloc(length);
	if (!newmem) return resultsList;
	SIZE_T bytesRead = 0;
	if (!ReadProcessMemory((HANDLE)-1, (void*)startAddress, (void*)newmem, length, &bytesRead))
	{
		free((void*)newmem);
		return resultsList;
	}

	for (uintptr_t i = newmem; i < newmem + length; i += 1)
	{
		uintptr_t realAddress = (i - newmem) + startAddress;
		uintptr_t candidate = *(DWORD*)i;
		candidate += realAddress + 4;
		if (candidate == (scanValue))
		{
			resultsList.push_back(realAddress);
		}
	}
	free((void*)newmem);
	return resultsList;
}
#else
std::vector<uintptr_t> FindCodeReferences(uintptr_t startAddress, size_t length, uintptr_t scanValue)
{
	std::vector<uintptr_t> resultsList;
	uintptr_t newmem = (uintptr_t)malloc(length);
	if (!newmem) return resultsList;
	SIZE_T bytesRead = 0;
	if (!ReadProcessMemory((HANDLE)-1, (void*)startAddress, (void*)newmem, length, &bytesRead))
	{
		free((void*)newmem);
		return resultsList;
	}

	for (uintptr_t i = newmem; i < newmem + length; i += 1)
	{
		uintptr_t candidate = *(uintptr_t*) i;
		if (candidate == (scanValue))
		{
			uintptr_t realAddress = (i - newmem) + startAddress;
			resultsList.push_back(realAddress);
		}
	}
	free((void*)newmem);
	return resultsList;
}
#endif