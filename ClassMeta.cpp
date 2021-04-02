#include "ClassMeta.h"

ClassMeta::ClassMeta(uintptr_t VTable, SectionInfo* sectionInfo)
{
	this->VTable = reinterpret_cast<uintptr_t*>(VTable);
	this->Meta = this->VTable - 1;
	COL = reinterpret_cast<CompleteObjectLocator*>(*Meta);
	
	pTypeDescriptor = COL->GetTypeDescriptor();
	pClassDescriptor = COL->GetClassDescriptor();
	pClassArray = pClassDescriptor->GetBaseClassArray();
	
	numBaseClasses = pClassDescriptor->numBaseClasses;
	
	bMultipleInheritance = (pClassDescriptor->attributes >> 0) & 1;
	bVirtualInheritance = (pClassDescriptor->attributes >> 1) & 1;
	bAmbigious = (pClassDescriptor->attributes >> 2) & 1;
	bInterface = false; // set by gui
	bStruct = false;

	if (*(&pTypeDescriptor->name + 3) == 'U') {
		bStruct = true;
	}
	VirtualFunctions = GetListOfFunctions((void*) VTable, sectionInfo);
	for (auto virtualFunction : VirtualFunctions) {
		BYTE* vf = reinterpret_cast<BYTE*>(virtualFunction);
		std::string vfName = "";
		if (vf[0] == 0xC3 || vf[0] == 0xC2)
			vfName += "nullsub_";
		else
			vfName += "sub_";
		vfName += IntegerToHexStr(virtualFunction);
		VirtualFunctionNames.push_back(vfName);
	}


	className = DemangleMicrosoft(&pTypeDescriptor->name);
	FilterSymbol(className);
	if (numBaseClasses >= 2) {
		for (unsigned int i = 1; i < numBaseClasses; i++) {
			BaseClassDescriptor* baseClass = GetBaseClass(i);
			char* ptr = &baseClass->GetTypeDescriptor()->name;
			baseClassNames.push_back(DemangleMicrosoft(ptr));
			FilterSymbol(baseClassNames[i - 1]);

			if (COL->offset == baseClass->where.mdisp) {
				interfaceName = DemangleMicrosoft(&baseClass->GetTypeDescriptor()->name);
				FilterSymbol(interfaceName);
			}
		}
	}

}

BaseClassDescriptor* ClassMeta::GetBaseClass(unsigned long index)
{
	return pClassArray->GetBaseClassDescriptor(index);
}
