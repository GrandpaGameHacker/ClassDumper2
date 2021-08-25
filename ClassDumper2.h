#pragma once
#include <Windows.h>
#include "GUI/Helpers/ImGuiApp.h"
#include "GUI/MainWindow.h"
#include "GUI/GuiState.h"

class ClassDumper2
{
public:
	ClassDumper2() {};
	ClassDumper2(ClassDumper2 const&) = delete;
	void operator=(ClassDumper2 const&) = delete;

	static DWORD __stdcall MainThread(void* lpParam);
	static void CleanExit();

	static HMODULE hSelf;
	static HWND DXWindow;
	static WNDCLASSEX WindowClass;

	static ImGuiApp DXApp;
};