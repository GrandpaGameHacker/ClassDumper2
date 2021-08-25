#include "ClassDumper2.h"

HMODULE ClassDumper2::hSelf;
HWND ClassDumper2::DXWindow;
WNDCLASSEX ClassDumper2::WindowClass;
ImGuiApp ClassDumper2::DXApp;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, &ClassDumper2::MainThread, hModule, NULL, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DWORD __stdcall ClassDumper2::MainThread(void* lpParam)
{
	hSelf = reinterpret_cast<HMODULE>(lpParam);

	WindowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, DXApp.WndProc, 0L, 0L, hSelf, NULL, NULL, NULL, NULL, "ClassDumper2", NULL };
	RegisterClassEx(&WindowClass);

	DXWindow = CreateWindow(
		WindowClass.lpszClassName,
		"ClassDumper 2", WS_OVERLAPPEDWINDOW,
		100, 100, 1280, 800, NULL, NULL,
		WindowClass.hInstance, NULL
	);

	if (!DXApp.CreateDeviceD3D(DXWindow)) {
		DXApp.CleanupDeviceD3D();
		DestroyWindow(DXWindow);
		UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
		FreeLibraryAndExitThread(hSelf, 1);
		return 1;
	}
	SetWindowLong(DXWindow, GWL_STYLE, 0);
	ShowWindow(DXWindow, SW_SHOWMAXIMIZED);
	UpdateWindow(DXWindow);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	ImGui::StyleColorsDark();
	DXApp.SetupBackend();
	GuiState::Init();
	GuiState::GuiLoop();

	CleanExit();
	return 0;
}

void ClassDumper2::CleanExit()
{
	DXApp.WaitForLastSubmittedFrame();
	DXApp.ShutdownBackend();
	DXApp.CleanupDeviceD3D();

	DestroyWindow(DXWindow);
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
	FreeLibraryAndExitThread(hSelf, 0);
}
