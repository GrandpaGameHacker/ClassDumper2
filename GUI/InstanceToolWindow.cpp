#include "InstanceToolWindow.h"
#include "GuiState.h"
void InstanceToolWindow::Draw()
{
    if (ImGui::Begin("Scanner Tool", 0, ImGuiWindowFlags_NoResize)) {
        if (ImGui::Button("Find Code References")) {
            GS::codeReferences.clear();
            GS::codeReferences = FindCodeReferences(GS::targetSectionInfo->TEXT.base, GS::targetSectionInfo->TEXT.size, (uintptr_t)GS::currentClass->VTable ^ 0xDEADBEEF);
        }
        ImGui::SameLine();
        if (ImGui::Button("Find Instances") && GS::currentClass) {
            GS::instances.clear();
            GS::instances = FindAllInstances((uintptr_t)GS::currentClass->VTable);
        }
        if (GS::instances.size() != 0) {
            ImGui::Text("Found %d instances in memory", GS::instances.size());
            for (unsigned int i = 0; i < GS::instances.size(); i++) {
                bool badVtable = false;
                if (GS::currentClass) {
                    if (!IsBadReadPointer((void*)GS::instances[i])) {
                        if (*(uintptr_t*)GS::instances[i] != ((uintptr_t)GS::currentClass->VTable ^ 0xDEADBEEF))
                        {
                            ImGui::TextColored({ 255,0,0,1 }, POINTER_FMTSTRING, GS::instances[i]);
                            if (ImGui::IsItemClicked()) {
                                GS::CopyToClipboard(POINTER_FMTSTRING, GS::instances[i]);
                            }
                            badVtable = true;
                        }
                    }
                    else {
                        ImGui::TextColored({ 255,0,0,1 }, POINTER_FMTSTRING, GS::instances[i]);
                        if (ImGui::IsItemClicked()) {
                            GS::CopyToClipboard(POINTER_FMTSTRING, GS::instances[i]);

                        }
                        badVtable = true;
                    }
                }
                if (!badVtable) {
                    ImGui::TextColored({ 0,255,0,1 }, POINTER_FMTSTRING, GS::instances[i]);
                    if (ImGui::IsItemClicked()) {
                        GS::CopyToClipboard(POINTER_FMTSTRING, GS::instances[i]);
                    }
                }
            }
        }
        if (GS::codeReferences.size() != 0) {
            ImGui::Text("Found %d vtable references in code", GS::codeReferences.size());
            for (unsigned int i = 0; i < GS::codeReferences.size(); i++) {
                ImGui::TextColored({ 0,255,225,1 }, POINTER_FMTSTRING, GS::codeReferences[i]);
                if (ImGui::IsItemClicked()) {
                    GS::CopyToClipboard(POINTER_FMTSTRING, GS::codeReferences[i]);
                }
            }
        }
        ImGui::SetWindowPos(WindowPos);
        ImGui::SetWindowSize(WindowSize);
        ImGui::End();
    }

}
