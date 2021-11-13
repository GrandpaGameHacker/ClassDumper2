#include "MainWindow.h"
#include "GuiState.h"
#include "..\ClassDumper2.h"
#include "..\Serialization\Serializer.h"

void MainWindow::Draw()
{
    if (ImGui::Begin(WindowTitle, 0, ImGuiWindowFlags_NoMove))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, DumpButtonColor);
        if (ImGui::Button("Dump!") && GS::targetModule != nullptr)
        {
            OnDumpButton();
        }
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Button, ImportButtonColor);
        ImGui::SameLine();
        if (ImGui::Button("Import")) {
            GS::importPopupState = true;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ExportButtonColor);
        ImGui::SameLine();
        if (ImGui::Button("Export")) {
            GS::exportPopupState = true;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ExitButtonColor);
        ImGui::SameLine();
        if (ImGui::Button("Exit")) {
            OnExitButton();
        }
        ImGui::PopStyleColor();
        ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
        if (ImGui::BeginCombo("##combo", GS::currentItem))
        {
            for (unsigned int n = 0; n < GS::modules.size(); n++) {
                bool is_selected = (GS::currentItem == GS::modules[n]->szModule);
                if (ImGui::Selectable(GS::modules[n]->szModule, is_selected)) {
                    GS::currentItem = GS::modules[n]->szModule;
                    GS::targetModule = GS::modules[n];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            GS::modules = GetModuleList(ClassDumper2::hSelf);
        }
        SearchBarRender();
        if (GS::exportPopupState) {
            GS::exportPopupState = PreExportPopup();
        }

        if (GS::importPopupState) {
            GS::importPopupState = PreImportPopup();
        }

        ImGui::SetWindowPos(WindowPos);
        ImGui::SetWindowSize(WindowSize);
        ImGui::End();
    }
}

void MainWindow::OnDumpButton()
{
    for (unsigned int i = 0; i < GS::VTables.size(); i++) {
        GS::VTables[i] = NULL;
    }
    GS::VTables.resize(0);
    GS::VTables.shrink_to_fit();
    GS::VTables.clear();
    GS::classes.clear();
    GS::targetSectionInfo = GetSectionInformation(GS::targetModule);
    if (GS::targetSectionInfo) {
        GS::bSectionInfoGood = true;
        GS::VTables= FindAllVTables(GS::targetSectionInfo);
        if (GS::VTables.size() != 0) {
            GS::bFoundVtables = true;
            Symbols::SortSymbols(GS::VTables);
            std::string lastClassName = "";
            for (unsigned int i = 0; i < GS::VTables.size(); i++)
            {
                auto cm = new ClassMetadata(GS::VTables[i], GS::targetSectionInfo);
                if (lastClassName == cm->className)
                {
                    if (cm->bMultipleInheritance) cm->bInterface = true;
                }
                else {
                    lastClassName = cm->className;
                }
                GS::classes.push_back(cm);
            }
        }
    }
    else {
        GS::bSectionInfoGood = false;
        GS::bFoundVtables = false;
    }
}

void MainWindow::SearchBarRender()
{
    if (ImGui::InputText("", GS::searchBuffer, 256)) {
        GS::searchClasses.clear();
        GS::bIsSearchActive = true;
        std::string searchString = GS::searchBuffer;
        if (GS::searchBuffer[0]) {
            for (auto cm : GS::classes) {
                std::string className = cm->className;
                StrLower(className);
                StrLower(searchString);
                if (className.find(searchString) != std::string::npos)
                {
                    GS::searchClasses.push_back(cm);
                }
            }
        }
        else {
            GS::bIsSearchActive = false;
        }
    }
}

void MainWindow::OnExitButton()
{
    GS::VTables.clear();
    GS::classes.clear();
    GS::modules.clear();
    ClassDumper2::CleanExit();
}

Serializer::ExportType exportType = Serializer::ExportType::TEXT;
static int exportState = 0;
bool MainWindow::PreExportPopup()
{

    if (GS::classes.empty()) {
        return false;
    }
    ImGui::OpenPopup("Export Results");
    if (ImGui::BeginPopupModal("Export Results")) {
        if (ImGui::RadioButton("Text format", &exportState, 0)) {
            exportState = 0;
            exportType = Serializer::ExportType::TEXT;
        }
        if (ImGui::RadioButton("XML format", &exportState, 1)) {
            exportState = 1;
            exportType = Serializer::ExportType::XML;
        };
        if (ImGui::Button("Save As...")) {
            std::string sFile = SaveDialog(exportType, ClassDumper2::DXWindow);
            if (!sFile.empty()) {
                ExportClasses(sFile, GS::classes, exportType);
            }
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

Serializer::ImportType importType = Serializer::ImportType::XML;
static int importState = 0;

bool MainWindow::PreImportPopup()
{
    if (GS::classes.empty()) {
        return false;
    }
    ImGui::OpenPopup("Import File");
    if (ImGui::BeginPopupModal("Import File")) {
        if (ImGui::RadioButton("XML format", &exportState, 1)) {
            importState = 1;
            importType = Serializer::ImportType::XML;
        };
        if (ImGui::Button("Open..")) {
            std::string sFile = OpenDialog(importType, ClassDumper2::DXWindow);
            if (!sFile.empty()) {
                ImportClasses(sFile, importType, GS::classes);
            }
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
