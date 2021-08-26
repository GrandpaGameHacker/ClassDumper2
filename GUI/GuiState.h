#pragma once
#include <Windows.h>
#include <vector>
#include "..\W32\Modules.h"
#include "..\W32\Disassembler.h"
#include "..\RTTI\ClassMetadata.h"

struct GuiState
{
	GuiState() {};
	GuiState(GuiState const&) = delete;
	void operator=(GuiState const&) = delete;
	static void Init();
	static void GuiLoop();
	static void CopyToClipboard(const char* format, ...);
	//MainWindow

	static std::vector<uintptr_t> VTables;
	static std::vector<ClassMetadata*> classes;
	static std::vector<ClassMetadata*> searchClasses;
	static std::vector< MODULEENTRY32*> modules;
	static MODULEENTRY32* targetModule;

	static char* currentItem;
	static char searchBuffer[256];
	static bool bIsSearchActive;

	static bool importPopupState;
	static bool exportPopupState;

	static bool bSectionInfoGood;
	static bool bFoundVtables;

	static SectionInfo* targetSectionInfo;

	//MainWindowEnd
	static Disassembler disasm;
	static std::vector<std::string> DisassembledInstructions;

	static ClassMetadata* currentClass;
	static std::vector<uintptr_t> instances;
	static std::vector<uintptr_t> codeReferences;

};

typedef GuiState GS;
