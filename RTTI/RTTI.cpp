#include "RTTI.h"
#ifdef _WIN64
uintptr_t RTTI::ModuleBase = 0;

RTTI::TypeDescriptor* RTTI::CompleteObjectLocator::GetTypeDescriptor()
{
	const auto CompleteObjectLocatorAddress = reinterpret_cast<uintptr_t>(&signature);
	ModuleBase = CompleteObjectLocatorAddress - CompleteObjectLocatorOffset;
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}

RTTI::ClassHierarchyDescriptor* RTTI::CompleteObjectLocator::GetClassDescriptor()
{
	return reinterpret_cast<ClassHierarchyDescriptor*>(ModuleBase + ClassDescriptorOffset);
}


RTTI::BaseClassArray* RTTI::ClassHierarchyDescriptor::GetBaseClassArray()
{
	return reinterpret_cast<BaseClassArray*>(ModuleBase + BaseClassArrayOffset);
}

RTTI::BaseClassDescriptor* RTTI::BaseClassArray::GetBaseClassDescriptor(unsigned long index)
{
	return reinterpret_cast<BaseClassDescriptor*>(ModuleBase + arrayOfBaseClassDescriptorOffsets[index]);
}

RTTI::TypeDescriptor* RTTI::BaseClassDescriptor::GetTypeDescriptor()
{
	return reinterpret_cast<TypeDescriptor*>(ModuleBase + TypeDescriptorOffset);
}

#else
RTTI::TypeDescriptor* RTTI::CompleteObjectLocator::GetTypeDescriptor() {
	return this->pTypeDescriptor;
}
RTTI::ClassHierarchyDescriptor* RTTI::CompleteObjectLocator::GetClassDescriptor()
{
	return this->pClassDescriptor;
}


RTTI::BaseClassArray* RTTI::ClassHierarchyDescriptor::GetBaseClassArray()
{
	return this->pBaseClassArray;
}

RTTI::BaseClassDescriptor* RTTI::BaseClassArray::GetBaseClassDescriptor(unsigned long index)
{
	return this->arrayOfBaseClassDescriptors[index];
}

RTTI::TypeDescriptor* RTTI::BaseClassDescriptor::GetTypeDescriptor()
{
	return this->pTypeDescriptor;
}
#endif


