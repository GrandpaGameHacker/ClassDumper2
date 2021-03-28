#pragma once
#include <Windows.h>
#include "ImGuiApp.h"
#include "ClassMeta.h"
#include "helpers/VTables.h"
#include "helpers/ModuleUtils.h"

DWORD WINAPI DllThread(void* lpParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HMODULE hModule;
static HWND DxWindow;
static WNDCLASSEX wc;
static ImGuiApp dxApp;

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


void MainGUI();
void ClassInspector();
void ExportData();
void RenderSceneDX12();
