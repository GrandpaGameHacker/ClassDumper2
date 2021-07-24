#pragma once
#include "../helpers/RTTI.h"
#include "../helpers/Symbols.h"
#include "../helpers/VTables.h"
#include "../helpers/StringConversions.h"

enum MemberType
{
	type_boolean,
	type_byte,
	type_word,
	type_signed_word,
	type_dword,
	type_signed_dword,
	type_qword,
	type_signed_qword,
	type_pointer,
	type_float,
	type_double,
	type_string
};

static const char* MemberType_str[] = {
	"bool", "byte", "word",
	"signed word", "dword",
	"signed dword", "qword",
	"signed qword", "pointer",
	"float", "double", "string" };

struct MemberVariable
{
	uintptr_t baseAddress = 0;
	uintptr_t offset = 0;
	MemberType type = MemberType::type_boolean;
	size_t size = 0;
	std::string name = "";
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

