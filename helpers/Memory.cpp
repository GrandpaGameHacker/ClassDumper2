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

// Multithreaded scanner, the first i've ever written
std::vector<uintptr_t> FindAllInstances(uintptr_t VTable)
{
	std::vector<uintptr_t> instances;
	if (!VTable) {
		return instances;
	}
	SYSTEM_INFO sysinfo;
	MEMORY_BASIC_INFORMATION mbi;
	::GetSystemInfo(&sysinfo);
	
	uintptr_t start = (uintptr_t) sysinfo.lpMinimumApplicationAddress;
	uintptr_t max = (uintptr_t)sysinfo.lpMaximumApplicationAddress;
	size_t pageSize = sysinfo.dwPageSize;
	std::vector<MEMORY_BASIC_INFORMATION*> mbiList;
	MEMORY_BASIC_INFORMATION* lastmbi = nullptr;
	for (uintptr_t current = start; current < max; current += mbi.RegionSize)
	{
		SIZE_T query = VirtualQuery((void*)current, &mbi, sizeof(mbi));
		if (!query) break;
		if (mbi.State != MEM_COMMIT || mbi.Protect & BadPageMask) continue;
		if (mbi.Protect == PAGE_READONLY) continue;
		auto p_mbi = new MEMORY_BASIC_INFORMATION();
		memcpy(p_mbi, &mbi, sizeof(mbi));
		mbiList.push_back(p_mbi);
	}
	bool notDone = true;
	unsigned int mbiIndex = 0;
	size_t mbiMax = mbiList.size();
	MEMORY_BASIC_INFORMATION* mbi0 = nullptr, *mbi1 = nullptr, *mbi2 = nullptr, *mbi3 = nullptr;
	std::future<std::vector<uintptr_t>> fut0, fut1, fut2, fut3;
	while (notDone) {
		if (mbiIndex == mbiMax) {
			notDone = false;
		}
		if (mbiIndex < mbiMax) {
			mbi0 = mbiList[mbiIndex];
			mbiIndex++;
		}

		if (mbiIndex < mbiMax) {
			mbi1 = mbiList[mbiIndex];
			mbiIndex++;
		}

		if (mbiIndex < mbiMax) {
			mbi2 = mbiList[mbiIndex];
			mbiIndex++;
		}

		if (mbiIndex < mbiMax) {
			mbi3 = mbiList[mbiIndex];
			mbiIndex++;
		}

		if (mbi0) {
			fut0 = std::async(FindReferences, (intptr_t)mbi0->BaseAddress, mbi0->RegionSize, VTable);
		}

		if (mbi1) {
			fut1 = std::async(FindReferences, (intptr_t)mbi1->BaseAddress, mbi1->RegionSize, VTable);
		}

		if (mbi2) {
			fut2 = std::async(FindReferences, (intptr_t)mbi2->BaseAddress, mbi2->RegionSize, VTable);
		}

		if (mbi3) {
			fut3 = std::async(FindReferences, (intptr_t)mbi3->BaseAddress, mbi3->RegionSize, VTable);
		}

		auto temp0 = fut0.get();
		auto temp1 = fut1.get();
		auto temp2 = fut2.get();
		auto temp3 = fut3.get();
		instances.insert(instances.end(), temp0.begin(), temp0.end());
		instances.insert(instances.end(), temp1.begin(), temp1.end());
		instances.insert(instances.end(), temp2.begin(), temp2.end());
		instances.insert(instances.end(), temp3.begin(), temp3.end());
	}
	mbiList.clear();
	return instances;
}

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
		if (candidate == scanValue)
		{
			uintptr_t realAddress = (i - newmem) + startAddress;
			resultsList.push_back(realAddress);
		}
	}
	free((void*)newmem);
	return resultsList;
}