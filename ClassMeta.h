#pragma once
#include "helpers/RTTI.h"
#include "helpers/Symbols.h"
#include "helpers/VTables.h"
#include "helpers/StringConversions.h"

enum class MemberType
{
	BOOLEAN,
	BYTE,
	WORD,
	DWORD,
	QWORD,
	POINTER,
	FLOAT,
	DOUBLE,
	STRING
};

struct MemberVariable
{
	uintptr_t baseAddress;
	uintptr_t offset;
	MemberType type;
};

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
	bool size_locked;
	std::vector<MemberVariable*> members;

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

