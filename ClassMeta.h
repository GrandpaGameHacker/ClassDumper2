#pragma once
#include "helpers/RTTI.h"
#include "helpers/Symbols.h"
#include "helpers/VTables.h"
#include "helpers/StringConversions.h"

struct ClassMeta
{
	//Functions
	explicit ClassMeta(uintptr_t VTable, SectionInfo* sectionInfo);
	BaseClassDescriptor* GetBaseClass(unsigned long index);

	//Members
	uintptr_t* VTable;
	std::vector<uintptr_t> VirtualFunctions;
	std::vector<std::string> VirtualFunctionNames;
	uintptr_t* Meta;

	size_t size;

	CompleteObjectLocator* COL;
	TypeDescriptor* pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor;
	BaseClassArray* pClassArray;


	bool bInterface;
	bool bStruct;

	bool bMultipleInheritance;
	bool bVirtualInheritance;
	bool bAmbigious;

	std::string className;
	unsigned long numBaseClasses;
	std::vector<std::string> baseClassNames;
	std::string interfaceName;
};

