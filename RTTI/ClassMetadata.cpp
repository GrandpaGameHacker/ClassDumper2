#include "ClassMetadata.h"

ClassMetadata::ClassMetadata(uintptr_t VTable, SectionInfo* sectionInfo)
{
	this->Meta = reinterpret_cast<uintptr_t*>(VTable) - 1;
	VTable ^= 0xDEADBEEF;
	this->VTable = reinterpret_cast<uintptr_t*>(VTable);
	COL = reinterpret_cast<RTTI::CompleteObjectLocator*>(*Meta);

	size = 0;
	size_locked = false;

	pTypeDescriptor = COL->GetTypeDescriptor();
	pClassDescriptor = COL->GetClassDescriptor();
	pClassArray = pClassDescriptor->GetBaseClassArray();

	numBaseClasses = pClassDescriptor->numBaseClasses;

	bMultipleInheritance = (pClassDescriptor->attributes >> 0) & 1;
	bVirtualInheritance = (pClassDescriptor->attributes >> 1) & 1;
	bAmbigious = (pClassDescriptor->attributes >> 2) & 1;

	bInterface = false;
	bStruct = false;

	if (*(&pTypeDescriptor->name + 3) == 'U') {
		bStruct = true;
	}
	
	VirtualFunctions = GetListOfFunctions((void*)this->VTable, sectionInfo);
	for (auto virtualFunction : VirtualFunctions) {
		BYTE* vf = reinterpret_cast<BYTE*>(virtualFunction);
		std::string vfName = "";
		if (*vf == 0xC3 || *vf == 0xC2)
			vfName += "nullsub_";
		else
			vfName += "sub_";
		vfName += IntegerToHexStr(virtualFunction);
		VirtualFunctionNames.push_back(vfName);
	}


	className = Symbols::DemangleMicrosoft(&pTypeDescriptor->name);
	Symbols::FilterSymbol(className);
	if (numBaseClasses >= 2) {
		for (unsigned int i = 1; i < numBaseClasses; i++) {
			RTTI::BaseClassDescriptor* baseClass = GetBaseClass(i);
			char* ptr = &baseClass->GetTypeDescriptor()->name;
			baseClassNames.push_back(Symbols::DemangleMicrosoft(ptr));
			Symbols::FilterSymbol(baseClassNames[i - 1]);

			if (COL->offset == baseClass->where.mdisp) {
				interfaceName = Symbols::DemangleMicrosoft(&baseClass->GetTypeDescriptor()->name);
				Symbols::FilterSymbol(interfaceName);
			}
		}
	}

}

RTTI::BaseClassDescriptor* ClassMetadata::GetBaseClass(unsigned long index)
{
	return pClassArray->GetBaseClassDescriptor(index);
}

