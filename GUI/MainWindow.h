#pragma once
#include "Interfaces/IWindow.h"

class MainWindow :
    public IWindow
{
    virtual void Draw() override;
    void OnDumpButton();
    void SearchBarRender();
    void OnExitButton();
    bool PreExportPopup();
    bool PreImportPopup();

    const char* WindowTitle = "ClassDumper 2.0";
    const ImVec2 WindowSize = ImVec2{ 900, 100 };
    const ImVec2 WindowPos = ImVec2{ 0,0 };

    const ImVec4 DumpButtonColor = ImVec4(ImColor(155, 0, 0));
    const ImVec4 ImportButtonColor = ImVec4(ImColor(0, 155, 125));
    const ImVec4 ExportButtonColor = ImVec4(ImColor(0, 155, 0));
    const ImVec4 ExitButtonColor = ImVec4(ImColor(0, 0, 155));
};

