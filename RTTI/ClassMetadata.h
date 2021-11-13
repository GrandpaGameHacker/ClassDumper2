#pragma once
#include "RTTI.h"
#include "Symbols.h"
#include "VTable.h"
#include "../Utility/StringConversion.h"

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
	type_string,
	type_vtable,
	type_error
};

static const char* MemberType_str[] =
{
	"bool", "unsigned char", "unsigned short",
	"signed short", "unsigned int",
	"signed int", "unsigned long long",
	"long long", "void*",
	"float", "double", "char", "void**", "error"
};

static const char* MemberTypeName_str[] =
{
	"bool", "uchar", "ushort",
	"short", "uint",
	"int", "ulonglong",
	"longlong", "ptr",
	"float", "double", "char",
	"vtable", "error"
};

struct MemberVariable
{
	uintptr_t baseAddress = 0;
	uintptr_t offset = 0;
	MemberType type = MemberType::type_boolean;
	size_t size = 0;
	std::string name = "";
	const char* stype = "";
};

struct ClassMetadata
{
	explicit ClassMetadata(uintptr_t VTable, SectionInfo* sectionInfo);
	RTTI::BaseClassDescriptor* GetBaseClass(unsigned long index);

	uintptr_t* VTable;
	std::vector<uintptr_t> VirtualFunctions;
	std::vector<std::string> VirtualFunctionNames;

	uintptr_t* Meta;
	RTTI::CompleteObjectLocator* COL;
	RTTI::TypeDescriptor* pTypeDescriptor;
	RTTI::ClassHierarchyDescriptor* pClassDescriptor;
	RTTI::BaseClassArray* pClassArray;

	size_t size;
	bool size_locked;
	std::vector<MemberVariable*> members;

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

