#pragma once
#include <Windows.h>
#include "gui/ImGuiApp.h"
#include "helpers/Exporter.h"
#include "helpers/Importer.h"
#include "helpers/ClassMeta.h"
#include "helpers/Disassembler.h"
#include "helpers/VTables.h"
#include "helpers/ModuleUtils.h"
#include "helpers/StringConversions.h"

#ifdef _WIN64
constexpr char POINTER_CLASSFMTSTR[] = "%s - VTable at %s";
constexpr char POINTER_METAFMTSTR[] = "CompleteObjectLocator: %llX";
constexpr char POINTER_FMTSTRING[] = "%llX";
constexpr char VTABLE_FMTSTRING[] = "%d - %llX - %s";
#else
constexpr char POINTER_CLASSFMTSTR[] = "%s - VTable at %s";
constexpr char POINTER_METAFMTSTR[] = "CompleteObjectLocator: %X";
constexpr char POINTER_FMTSTRING[] = "%X";
constexpr char VTABLE_FMTSTRING[] = "%d - %X  - %s";
#endif

DWORD WINAPI DllThread(void* lpParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static ImGuiWindowFlags flags;
static HMODULE hModule;
static HWND DxWindow;
static WNDCLASSEX wc;
static ImGuiApp dxApp;
static SectionInfo* targetSectionInfo;

static bool bSectionInfoGood = false;
static bool bFoundVtables = false;
static char* currentItem;

static std::vector<MODULEENTRY32*> modules;
static std::vector<uintptr_t> VtableList;
static std::vector<ClassMeta*> classes;

static MODULEENTRY32* targetModule;
static ClassMeta* currentClass;

static char searchBuffer[256];
static bool bIsSearchActive;
static std::vector<ClassMeta*> searchClasses;

static std::vector<uintptr_t> instances;
static std::vector<uintptr_t> codeReferences;

static Disassembler disasm = Disassembler();
static std::vector<std::string> DisassembledInstructions;

void MainGUI();
void Dump();
void ClassViewer();
void ClassInspector();
bool RenameVFunctionPopup(std::string& functionName);
bool DisassembleFunctionPopup();
bool PreExportPopup();
bool PreImportPopup();
void InstanceTool();
void AutoStructureDissect();
bool StructureDissectWindow();
MemberType AutoGuessType(uintptr_t address);