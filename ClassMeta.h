#pragma once
#include "helpers/RTTI.h"
#include "helpers/Symbols.h"

struct ClassMeta
{
	//Functions
	explicit ClassMeta(uintptr_t VTable);
	BaseClassDescriptor* GetBaseClass(unsigned long index);

	//Members
	uintptr_t* VTable;
	uintptr_t* Meta;

	CompleteObjectLocator* COL;
	TypeDescriptor* pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor;
	BaseClassArray* pClassArray;

	unsigned long numBaseClasses;

	bool bMultipleInheritance;
	bool bVirtualInheritance;
	bool bAmbigious;

	std::string className;
	std::vector<std::string> baseClassNames;
};

