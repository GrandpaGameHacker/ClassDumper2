#include "ClassViewerWindow.h"
#include "GuiState.h"

void ClassViewerWindow::Draw()
{
    auto localClasses = GS::classes;
    if (ImGui::Begin(WindowTitle, 0, ImGuiWindowFlags_NoResize))
    {
        if (GS::bSectionInfoGood && GS::bFoundVtables)
        {
            if (GS::bIsSearchActive) {
                localClasses = GS::searchClasses;
            }
            for (auto classMetadata : localClasses)
            {
                std::string fmt;
                if (classMetadata->bInterface)
                {
                    fmt += _Interface;
                }
                else if (classMetadata->bStruct)
                    fmt += _Struct;
                else
                    fmt += _Class;
                if (classMetadata->bInterface)
                    ImGui::Text(fmt.c_str(), classMetadata->interfaceName.c_str());
                else
                    ImGui::Text(fmt.c_str(), classMetadata->className.c_str());
                if (ImGui::IsItemClicked())
                {
                    GS::instances.clear();
                    GS::codeReferences.clear();
                    GS::currentClass = classMetadata;
                }
            }

        }
        ImGui::SetWindowPos(WindowPos);
        ImGui::SetWindowSize(WindowSize);
        ImGui::End();
    }
}
