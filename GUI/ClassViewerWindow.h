#pragma once
#include "Interfaces/IWindow.h"

class ClassViewerWindow : public IWindow
{
	virtual void Draw() override;

	const char* WindowTitle = "ClassViewer";
	const char* _Struct = "struct %s";
	const char* _Interface = "\tinterface : %s";
	const char* _Class = "class %s";
	const ImVec2 WindowPos = ImVec2{ 0,101 };
	const ImVec2 WindowSize = ImVec2{ 900,975 };
};

