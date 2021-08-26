#pragma once

#include "./Interfaces/IWindow.h"
class InstanceToolWindow :
    public IWindow
{
    void Draw() override;

	const ImVec2 WindowPos = ImVec2{ 1500,0 };
	const ImVec2 WindowSize = ImVec2{ 600,1075 };

#ifdef _WIN64
	const char* POINTER_FMTSTRING = "%llX";
	const char* VTABLE_FMTSTRING = "%d - %llX - %s";
#else
	const char* POINTER_FMTSTRING = "%X";
	const char* VTABLE_FMTSTRING = "%d - %X  - %s";
#endif
};