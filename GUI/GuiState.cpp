#include "GuiState.h"
#include "../ClassDumper2.h"
#include "MainWindow.h"
#include "ClassViewerWindow.h"
#include "ClassInspectorWindow.h"
#include "InstanceToolWindow.h"

std::vector <uintptr_t> GS::VTables;
std::vector <ClassMetadata*> GS::classes;
std::vector <ClassMetadata*> GS::searchClasses;
std::vector <MODULEENTRY32*> GS::modules;
MODULEENTRY32* GS::targetModule;

char* GS::currentItem;
char GS::searchBuffer[256] = {};
bool GS::bIsSearchActive = false;

bool GS::bSectionInfoGood = false;
bool GS::bFoundVtables = false;
bool GS::exportPopupState = false;
bool GS::importPopupState = false;

SectionInfo* GS::targetSectionInfo;

ClassMetadata* GS::currentClass;
std::vector<uintptr_t> GS::instances;
std::vector<uintptr_t> GS::codeReferences;

std::vector<std::string> GS::DisassembledInstructions;

Disassembler GS::disasm = Disassembler();


static bool bCreated = false;
void GuiState::Init()
{
    if(!bCreated){
        
        bCreated = true;
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
        static MainWindow mainWindow = MainWindow();
        static ClassViewerWindow classViewerWindow = ClassViewerWindow();
        static ClassInspectorWindow classWindow = ClassInspectorWindow();
        static InstanceToolWindow instanceWindow = InstanceToolWindow();


        mainWindow.Enable();
        classViewerWindow.Enable();
        classWindow.Enable();
        instanceWindow.Enable();

        strcpy_s(searchBuffer, "search here...");
        modules = GetModuleList(ClassDumper2::hSelf);
        currentItem = modules[0]->szModule;
        targetModule = modules[0];
        mainWindow.OnDumpButton();
    }
}

void GuiState::GuiLoop()
{
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        ClassDumper2::DXApp.CreateFrame();
        for (IWindow* window : IWindow::WindowList) {
            window->Tick();
        }
        ClassDumper2::DXApp.RenderFrame();
    }

}

void GuiState::CopyToClipboard(const char* format, ...)
{
    char buffer[1024] = { 0 };
    va_list v1;
    va_start(v1, format);
    vsnprintf(buffer, 1024, format, v1);
    va_end(v1);
    ImGui::SetClipboardText(buffer);
}
