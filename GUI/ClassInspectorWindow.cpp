#include "ClassInspectorWindow.h"
#include "GuiState.h"

bool disassemblerState = false;
bool structureDissectState = false;
bool renamingState = false;
char renameBuffer[256] = { 0 };
unsigned int vfindex = 0;

void ClassInspectorWindow::Draw()
{
    if (ImGui::Begin("Class Info"))
    {
        if (!GS::currentClass)
            ImGui::Text("Class not selected yet.");
        else
        {
            std::string VTableString = IntegerToHexStr((uintptr_t)GS::currentClass->VTable ^ 0xDEADBEEF);
            ImGui::Text("%s - VTable at %s", GS::currentClass->className.c_str(), VTableString.c_str());
            if (ImGui::IsItemClicked())
                GS::CopyToClipboard("%s - %s", VTableString.c_str(), GS::currentClass->className.c_str());

            ImGui::Text(POINTER_METAFMTSTR, (uintptr_t)GS::currentClass->Meta);
            if (ImGui::IsItemClicked())
                GS::CopyToClipboard(POINTER_FMTSTRING, (uintptr_t)GS::currentClass->Meta);

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({ 255,0,0,1 }, "Inheritance Tree");
            ImGui::Separator();

            if (GS::currentClass->numBaseClasses >= 2) {
                unsigned int tabIndex = 0;
                unsigned int lastOffset = -1;
                for (unsigned int i = 0; i < GS::currentClass->baseClassNames.size(); i++) {
                    if (lastOffset == GS::currentClass->GetBaseClass(i + 1)->where.mdisp) {
                        tabIndex += 1;
                    }
                    else {
                        lastOffset = GS::currentClass->GetBaseClass(i + 1)->where.mdisp;
                        tabIndex = 0;
                    }
                    std::string formatString;
                    for (unsigned int x = 0; x < tabIndex; x++) { formatString.append("\t"); }
                    formatString.append("%s");
                    ImGui::Text(formatString.c_str(), GS::currentClass->baseClassNames[i].c_str());
                }
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({ 0,120,250,1 }, "Information");
            ImGui::Separator();
            if (!GS::currentClass->bInterface) {
                if (GS::currentClass->bVirtualInheritance)
                    ImGui::Text("Has Virtual Inheritance");
                if (GS::currentClass->bMultipleInheritance)
                    ImGui::Text("Has Multiple Inheritance");
                if (GS::currentClass->bAmbigious)
                    ImGui::Text("Has Ambigious Inheritance");
            }
            else {
                ImGui::Text("This is an interface implementation");
                ImGui::Text("Implementing: %s", GS::currentClass->interfaceName.c_str());
            }
            ImGui::Text("Number of parents: %d", GS::currentClass->numBaseClasses - 1);
            ImGui::Text("Number of virtual functions: %d", GS::currentClass->VirtualFunctions.size());
            ImGui::Text("Size Of Class: "); ImGui::SameLine();
            if (GS::currentClass->size_locked) {
                ImGui::TextDisabled("%08X", GS::currentClass->size);
            }
            else {
                ImGui::PushItemWidth(65.0f);
                ImGui::InputScalar("", ImGuiDataType_U64, &GS::currentClass->size, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::PopItemWidth();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Lock Size", &GS::currentClass->size_locked);

            if (ImGui::Button("Auto Dissect")) {
               /* AutoStructureDissect();
                structureDissectState = true;*/

            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({ 250,100,0,1 }, "Virtual Functions:");
            ImGui::Separator();
            {
                bool vfmenuState = false;
                auto vfs = GS::currentClass->VirtualFunctions;

                for (unsigned int i = 0; i < vfs.size(); i++) {

                    ImGui::TextColored({ 250,0,130,1 }, VTABLE_FMTSTRING, i, vfs[i], GS::currentClass->VirtualFunctionNames[i].c_str());

                    if (ImGui::IsItemClicked())
                        GS::CopyToClipboard(VTABLE_FMTSTRING, i, vfs[i], GS::currentClass->VirtualFunctionNames[i].c_str());

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        vfindex = i;

                    if (!vfmenuState) {
                        if (ImGui::BeginPopupContextItem("FunctionContext"))
                        {
                            if (ImGui::MenuItem("Disassemble Function"))
                            {
                                uintptr_t func = GS::currentClass->VirtualFunctions[vfindex];
                                size_t funcSize = GS::disasm.GetFunctionSize(func);
                                GS::DisassembledInstructions = GS::disasm.DecodeToString((uint8_t*)func, funcSize);
                                disassemblerState = true;
                            }
                            if (ImGui::MenuItem("Rename Function")) {
                                memcpy_s(renameBuffer, 256, GS::currentClass->VirtualFunctionNames[vfindex].c_str(), 256);
                                renamingState = true;
                            }
                            ImGui::EndPopup();
                            vfmenuState = true;
                        }
                    }
                }
            }
        }
        if (renamingState) {
            renamingState = RenameVFunctionPopup(GS::currentClass->VirtualFunctionNames[vfindex]);
        }
        if (disassemblerState) {
            disassemblerState = DisassembleFunctionPopup();
        }
        if (structureDissectState) {
            structureDissectState = StructureDissectWindow();
        }

        ImGui::SetWindowPos(ImVec2{ 900,0 });
        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::End();
    }
}

bool ClassInspectorWindow::RenameVFunctionPopup(std::string &functionName)
{

    ImGui::OpenPopup("Rename Function");
    if (ImGui::BeginPopupModal("Rename Function", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("", renameBuffer, 256);
        if (ImGui::Button("Rename")) {
            functionName = std::string(renameBuffer);
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool ClassInspectorWindow::DisassembleFunctionPopup()
{
    ImGui::OpenPopup("Disassemble Function");
    if (ImGui::BeginPopupModal("Disassemble Function")) {
        if (ImGui::BeginChild("DisDisplay", { 500,500 })) {
            for (auto instr : GS::DisassembledInstructions) {
                ImGui::Text(instr.c_str());
            }
            ImGui::EndChild();
        }
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }


        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool ClassInspectorWindow::StructureDissectWindow()
{
    if (GS::currentClass->members.size() == 0) return false;

    if (ImGui::Begin("Structure Dissect")) {

        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::Text(POINTER_FMTSTRING, GS::currentClass->members[0]->baseAddress);
        for (auto member : GS::currentClass->members) {
            /*ImGui::Text("%d | %s | %s", member->offset, member->name, MemberType_str[member->type]);*/
        }
        ImGui::End();
    }
    return true;
}
