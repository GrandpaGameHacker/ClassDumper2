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

static bool bSectionInfoGood = false;
static bool bFoundVtables = false;
static char* currentItem;

static std::vector<MODULEENTRY32*> modules;
static MODULEENTRY32* targetModule;
static std::vector<uintptr_t> VtableList;
static std::vector<ClassMeta*> classes;
static ClassMeta* currentClass;

void MainGUI();
void ClassInspector();
void RenderSceneDX12();
