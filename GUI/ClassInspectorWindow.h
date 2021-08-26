#pragma once
#include "Interfaces/IWindow.h"
#include <string>

class ClassInspectorWindow :
	public IWindow
{
	void Draw() override;
	bool RenameVFunctionPopup(std::string &functionName);
	bool DisassembleFunctionPopup();
	bool StructureDissectWindow();
#ifdef _WIN64
	const char* POINTER_METAFMTSTR = "CompleteObjectLocator: %llX";
	const char* POINTER_FMTSTRING = "%llX";
	const char* VTABLE_FMTSTRING = "%d - %llX - %s";
#else
	const char* POINTER_METAFMTSTR = "CompleteObjectLocator: %X";
	const char* POINTER_FMTSTRING = "%X";
	const char* VTABLE_FMTSTRING = "%d - %X  - %s";
#endif
};

