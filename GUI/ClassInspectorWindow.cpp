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
               AutoStructureDissect();
               structureDissectState = true;

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

        ImGui::SetWindowPos(WindowPos);
        ImGui::SetWindowSize(WindowSize);
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

void ClassInspectorWindow::AutoStructureDissect()
{
    auto size = GS::currentClass->size;
    if (size == 0) return;
    if (GS::instances.size() == 0) return;

    GS::currentClass->members.clear();
    unsigned int i = 0;
    size_t offset = 0;
    while (1)
    {
        if (offset >= size) break;

        auto address = GS::instances[i] + offset;
        size_t member_size = 0;
        MemberType type = AutoGuessMember(address);

        switch (type)
        {
        case MemberType::type_boolean:
        case MemberType::type_byte:
            member_size = sizeof(type_byte);
            break;

        case MemberType::type_signed_dword:
        case MemberType::type_dword:
        case MemberType::type_float:
            member_size = sizeof(float);
            break;

        case MemberType::type_signed_qword:
        case MemberType::type_qword:
        case MemberType::type_double:
            member_size = sizeof(double);
            break;

        case MemberType::type_pointer:
            member_size = sizeof(uintptr_t);
            break;

        case MemberType::type_string:
            member_size = strlen((const char*)address);
            break;

        default:
            type = MemberType::type_byte;
            member_size = 1;
            break;
        }

        MemberVariable* member = new MemberVariable();
        member->baseAddress = GS::instances[i];
        member->offset = offset;
        member->type = type;
        member->size = member_size;
        member->stype = MemberType_str[member->type];
        GS::currentClass->members.push_back(member);
        offset += member_size;
    }
}

MemberType ClassInspectorWindow::AutoGuessMember(uintptr_t address)
{
    float f = *(float*)address;
    double db = *(double*)address;
    unsigned long long qw = *(unsigned long long*) address;
    uintptr_t ptr = *(uintptr_t*)address;

    if (qw == 0)
        return MemberType::type_dword;

    if (!IsBadReadPointer((void*)ptr))
        if (address % sizeof(uintptr_t) != 1)
            return MemberType::type_pointer;

    if (std::isnormal(f) && f > 0.0001 && f < 100000)
        return MemberType::type_float;

    if (std::isnormal(db) && db > 0.0001 && db < 100000)
        return MemberType::type_double;

    if (std::string((char*)address).size() > 5)
        return MemberType::type_string;

    return MemberType::type_dword;
}

bool ClassInspectorWindow::StructureDissectWindow()
{
    if (GS::currentClass->members.size() == 0) return false;

    if (ImGui::Begin("Structure Dissect")) {

        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::Text(POINTER_FMTSTRING, GS::currentClass->members[0]->baseAddress);
        for (auto member : GS::currentClass->members) {
        // broken    ImGui::Text("%d | %s | %s", member->offset, member->name.c_str(), member->stype);
        }
        ImGui::End();
    }
    return true;
}
