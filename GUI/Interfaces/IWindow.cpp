#include "IWindow.h"
std::vector<IWindow*> IWindow::WindowList;
IWindow::IWindow()
{
    static size_t WindowIDCount = 0;
    WindowID = WindowIDCount++;
    IWindow::WindowList.push_back(this);
}

IWindow::~IWindow() {
    for (size_t i = 0; i < IWindow::WindowList.size(); i++)
    {
        if (IWindow::WindowList[i] == this)
        {
            IWindow::WindowList.erase(IWindow::WindowList.begin() + i);
        };
    }
}

bool IWindow::operator==(const IWindow& RHS)
{
        return WindowID == RHS.WindowID;
}

void IWindow::Tick()
{
    if (bEnabled) {
        Draw();
    }
}

bool IWindow::IsShown()
{
    return bEnabled;
}

bool IWindow::Enable()
{
    bEnabled = true;
    return true;
}

bool IWindow::Disable()
{
    bEnabled = false;
    return false;
}
