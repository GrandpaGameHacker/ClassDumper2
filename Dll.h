#pragma once
#include <Windows.h>
#include "ImGuiApp.h"
#include "ClassMeta.h"
#include "helpers/VTables.h"
#include "helpers/ModuleUtils.h"
#include "helpers/StringConversions.h"

#ifdef _WIN64
constexpr char POINTER_CLASSFMTSTR[] = "%s - VTable at %llX";
constexpr char POINTER_METAFMTSTR[] = "CompleteObjectLocator: %llX";
constexpr char POINTER_FMTSTRING[] = "%llX";
constexpr char VTABLE_FMTSTRING[] = "%d - %llX - %s";
#else
constexpr char POINTER_CLASSFMTSTR[] = "%s - VTable at %X";
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

void MainGUI();
void ClassViewer();
void ClassInspector();
void InstanceTool();
void ExportData();
void RenderSceneDX12();