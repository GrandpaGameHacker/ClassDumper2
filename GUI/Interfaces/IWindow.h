#pragma once
#include "imgui.h"
#include <vector>
class IWindow
{
public:
	IWindow();
	virtual ~IWindow();
	bool operator==(const IWindow& RHS);
	virtual void Tick();
	virtual void Draw() = 0;
	virtual bool IsShown();
	virtual bool Enable();
	virtual bool Disable();
	static std::vector<IWindow*> WindowList;
private:
	size_t WindowID;
	bool bEnabled = false;
};

