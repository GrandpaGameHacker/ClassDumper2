#include "ClassMeta.h"

ClassMeta::ClassMeta(uintptr_t VTable)
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

	className = DemangleMicrosoft(&pTypeDescriptor->name);
	FilterSymbol(className);
	if (numBaseClasses >= 2) {
		for (unsigned int i = 1; i < numBaseClasses; i++) {
			char* ptr = &GetBaseClass(i)->GetTypeDescriptor()->name;
			baseClassNames.push_back(DemangleMicrosoft(ptr));
			FilterSymbol(baseClassNames[i-1]);
		}
	}
}

BaseClassDescriptor* ClassMeta::GetBaseClass(unsigned long index)
{
	return pClassArray->GetBaseClassDescriptor(index);
}
