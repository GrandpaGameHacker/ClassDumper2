#include "Dll.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, &DllThread, hModule, NULL, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DWORD WINAPI DllThread(void* lpParam)
{
    // Create our window
    hModule = reinterpret_cast<HMODULE>(lpParam);
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hModule, NULL, NULL, NULL, NULL, "ClassDumper2", NULL };
    ::RegisterClassEx(&wc);
    DxWindow = ::CreateWindow(wc.lpszClassName, "ClassDumper 2.0", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Setup directX 12 and window
    if (!dxApp.CreateDeviceD3D(DxWindow)) {
        dxApp.CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        ::FreeLibraryAndExitThread(hModule, 1);
        return 1;
    }
    ::SetWindowLong(DxWindow, GWL_STYLE, 0);
    ::ShowWindow(DxWindow, SW_SHOWMAXIMIZED);
    ::UpdateWindow(DxWindow);

    // Setup ImGui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    dxApp.SetupBackend();

    modules = GetModuleList(hModule);
    bool done = false;
    flags = ImGuiWindowFlags_NoMove;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;
        dxApp.CreateFrame();
        {
            // Render imgui windows and define logic
            MainGUI();
            ClassViewer();
            ClassInspector();
            InstanceTool();
        }
        dxApp.RenderFrame();
    }
        dxApp.WaitForLastSubmittedFrame();

        // Cleanup
        dxApp.ShutdownBackend();
        dxApp.CleanupDeviceD3D();
        ::DestroyWindow(DxWindow);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        ::FreeLibraryAndExitThread(hModule, 0);
        return 0;
}


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (dxApp.m_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            dxApp.WaitForLastSubmittedFrame();
            ImGui_ImplDX12_InvalidateDeviceObjects();
            dxApp.CleanupRenderTarget();
            dxApp.ResizeSwapChain((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
            dxApp.CreateRenderTarget();
            ImGui_ImplDX12_CreateDeviceObjects();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool exportPopupState = false;
bool importPopupState = false;
void MainGUI()
{
    if (ImGui::Begin("Class Dumper 2.0", 0, flags))
    {
        ImGui::Text("Target Module:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##combo", currentItem))
        {
            for (unsigned int n = 0; n < modules.size(); n++) {
                bool is_selected = (currentItem == modules[n]->szModule);
                if (ImGui::Selectable(modules[n]->szModule, is_selected)) {
                    currentItem = modules[n]->szModule;
                    targetModule = modules[n];
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            modules = GetModuleList(hModule);
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(155, 0, 0)));
        if (ImGui::Button("Dump!") && targetModule != nullptr)
        {
            Dump();
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 155, 125)));
        ImGui::SameLine();
        if (ImGui::Button("Import")) {
            importPopupState = true;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 155, 0)));
        ImGui::SameLine();
        if (ImGui::Button("Export")) {
            exportPopupState = true;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 0, 155)));
        ImGui::SameLine();
        if (ImGui::Button("Exit")) {

            VtableList.clear();
            classes.clear();
            modules.clear();

            dxApp.WaitForLastSubmittedFrame();
            dxApp.ShutdownBackend();
            dxApp.CleanupDeviceD3D();

            ::DestroyWindow(DxWindow);
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            FreeLibraryAndExitThread(hModule, 0);
        }
        ImGui::PopStyleColor();
        ImGui::Text("Search bar:"); ImGui::SameLine();
        if (ImGui::InputText("", searchBuffer, 256)) {
            searchClasses.clear();
            bIsSearchActive = true;
            std::string searchString = searchBuffer;
            if (searchBuffer[0]) {
                for (auto cm : classes) {
                    std::string className = cm->className;
                    StrLower(className);
                    StrLower(searchString);
                    if (className.find(searchString) != std::string::npos)
                    {
                        searchClasses.push_back(cm);
                    }
                }
            }
            else {
                bIsSearchActive = false;
            }
        }
        if (exportPopupState) {
            exportPopupState = PreExportPopup();
        }

        if (importPopupState) {
            importPopupState = PreImportPopup();
        }

        ImGui::SetWindowPos(ImVec2{ 0,0 });
        ImGui::SetWindowSize(ImVec2{900,100});
        ImGui::End();
    }
}

void Dump()
{
    VtableList.clear();
    classes.clear();
    targetSectionInfo = GetSectionInformation(targetModule);
    if (targetSectionInfo) {
        bSectionInfoGood = true;
        VtableList = FindAllVTables(targetSectionInfo);
        if (VtableList.size() != 0) {
            bFoundVtables = true;
            SortSymbols(VtableList);
            std::string lastClassName = "";
            for (unsigned int i = 0; i < VtableList.size(); i++)
            {
                auto cm = new ClassMeta(VtableList[i], targetSectionInfo);
                if (lastClassName == cm->className)
                {
                    if (cm->bMultipleInheritance) cm->bInterface = true;
                }
                else {
                    lastClassName = cm->className;
                }
                classes.push_back(cm);
            }
        }
    }
    else {
        bSectionInfoGood = false;
        bFoundVtables = false;
    }
    for (unsigned int i = 0; i < VtableList.size(); i++) {
        VtableList[i] = NULL;
    }
    VtableList.resize(0);
    VtableList.shrink_to_fit();
}

void ClassViewer() {
    auto localClasses = classes;
    if (ImGui::Begin("ClassView", 0, flags)) {
        if (bSectionInfoGood && bFoundVtables) {
            if (bIsSearchActive) {
                localClasses = searchClasses;
            }
            for (auto cm : localClasses) {
                std::string fmt = "";
                if (cm->bInterface)
                {
                    fmt += "interface %s -> %s";
                }
                else if (cm->bStruct)
                    fmt += "struct %s";
                else
                    fmt += "class %s";
                if (cm->bInterface)
                    ImGui::Text(fmt.c_str(), cm->className.c_str(), cm->interfaceName.c_str());
                else
                    ImGui::Text(fmt.c_str(), cm->className.c_str());
                if (ImGui::IsItemClicked())
                {
                    instances.clear();
                    codeReferences.clear();
                    currentClass = cm;
                }
            }

        }
        ImGui::SetWindowPos(ImVec2{ 0,101 });
        ImGui::SetWindowSize(ImVec2{ 900,950 });
        ImGui::End();
    }
}

bool disassemblerState = false;
bool structureDissectState = false;
bool renamingState = false;
char renameBuffer[256] = { 0 };
unsigned int vfindex = 0;

void ClassInspector()
{
    if (ImGui::Begin("Class Info"))
    {
        if (!currentClass)
            ImGui::Text("Class not selected yet.");
        else
        {
            std::string VTableString = IntegerToHexStr((uintptr_t)currentClass->VTable ^ 0xDEADBEEF);
            ImGui::Text(POINTER_CLASSFMTSTR, currentClass->className.c_str(), VTableString.c_str());
            if (ImGui::IsItemClicked())
            {
                char buffer[256] = { 0 };
                sprintf_s(buffer, POINTER_CLASSFMTSTR, currentClass->className.c_str(), VTableString.c_str());

                ImGui::SetClipboardText(buffer);
            }
            ImGui::Text(POINTER_METAFMTSTR, (uintptr_t)currentClass->Meta);
            if (ImGui::IsItemClicked()) {
                char buffer[256] = { 0 };
                sprintf_s(buffer, POINTER_METAFMTSTR, (uintptr_t)currentClass->Meta);

                ImGui::SetClipboardText(buffer);
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({255,0,0,1}, "Inheritance Tree");
            ImGui::Separator();

            if (currentClass->numBaseClasses >= 2) {
                unsigned int tabIndex = 0;
                unsigned int lastOffset = -1;
                for (unsigned int i = 0; i < currentClass->baseClassNames.size(); i++) {
                    if (lastOffset == currentClass->GetBaseClass(i + 1)->where.mdisp) {
                        tabIndex += 1;
                    }
                    else {
                        lastOffset = currentClass->GetBaseClass(i + 1)->where.mdisp;
                        tabIndex = 0;
                    }
                    std::string formatString;
                    for (unsigned int x = 0; x < tabIndex; x++) { formatString.append("\t"); }
                    formatString.append("%s");
                    ImGui::Text(formatString.c_str(), currentClass->baseClassNames[i].c_str());
                }
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({0,120,250,1}, "Information");
            ImGui::Separator();
            if (!currentClass->bInterface) {
                if (currentClass->bVirtualInheritance)
                    ImGui::Text("Has Virtual Inheritance");
                if (currentClass->bMultipleInheritance)
                    ImGui::Text("Has Multiple Inheritance");
                if (currentClass->bAmbigious)
                    ImGui::Text("Has Ambigious Inheritance");
            }
            else {
                ImGui::Text("This is an interface implementation");
                ImGui::Text("Implementing: %s", currentClass->interfaceName.c_str());
            }
            ImGui::Text("Number of parents: %d", currentClass->numBaseClasses -1);
            ImGui::Text("Number of virtual functions: %d", currentClass->VirtualFunctions.size());
            ImGui::Text("Size Of Class: "); ImGui::SameLine();
            if (currentClass->size_locked) {
                ImGui::TextDisabled("%08X", currentClass->size);
            }
            else {
                ImGui::PushItemWidth(65.0f);
                ImGui::InputScalar("", ImGuiDataType_U64, &currentClass->size, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::PopItemWidth();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Lock Size", &currentClass->size_locked);

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
                auto vfs = currentClass->VirtualFunctions;
                for (unsigned int i = 0; i < vfs.size(); i++) {
                    ImGui::TextColored({ 250,0,130,1 }, VTABLE_FMTSTRING, i, vfs[i], currentClass->VirtualFunctionNames[i].c_str());
                    if (ImGui::IsItemClicked()) {
                        char buffer[256] = { 0 };
                        sprintf_s(buffer, VTABLE_FMTSTRING, i, vfs[i], currentClass->VirtualFunctionNames[i].c_str());
                        ImGui::SetClipboardText(buffer);
                    }
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                        vfindex = i;
                    }
                    if (!vfmenuState) {
                        if (ImGui::BeginPopupContextItem("FunctionContext"))
                        {
                            if(ImGui::MenuItem("Disassemble Function"))
                            {
                                uintptr_t func = currentClass->VirtualFunctions[vfindex];
                                size_t funcSize = disasm.GetFunctionSize(func);
                                DisassembledInstructions = disasm.DecodeToString((uint8_t*)func, funcSize);
                                disassemblerState = true;
                            }
                            if (ImGui::MenuItem("Rename Function")) {
                                memcpy_s(renameBuffer, 256, currentClass->VirtualFunctionNames[vfindex].c_str(), 256);
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
            renamingState = RenameVFunctionPopup(currentClass->VirtualFunctionNames[vfindex]);
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


bool RenameVFunctionPopup(std::string& functionName)
{
    
    ImGui::OpenPopup("Rename Function");
    if (ImGui::BeginPopupModal("Rename Function", 0, ImGuiWindowFlags_AlwaysAutoResize)){
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

bool DisassembleFunctionPopup()
{
    ImGui::OpenPopup("Disassemble Function");
    if (ImGui::BeginPopupModal("Disassemble Function")) {
        if (ImGui::BeginChild("DisDisplay", {500,500})) {
             for (auto instr : DisassembledInstructions) {
                        ImGui::Text(instr.c_str());
                    }
             ImGui::EndChild();
        }
        if (ImGui::Button("Close")){
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }
       

        ImGui::EndPopup();
        return true;
    }
    return false;
}

ExportType exportType = ExportType::TEXT;
static int exportState = 0;
bool PreExportPopup()
{

    if (classes.empty()) {
        return false;
    }
    ImGui::OpenPopup("Export Results");
    if (ImGui::BeginPopupModal("Export Results")) {
        if (ImGui::RadioButton("Text format", &exportState, 0)) {
            exportState = 0;
            exportType = ExportType::TEXT;
        }
        if (ImGui::RadioButton("XML format", &exportState, 1)) {
            exportState = 1;
            exportType = ExportType::XML;
        };
        if (ImGui::Button("Save As...")) {
            std::string sFile = SaveDialog(exportType, DxWindow);
            if (!sFile.empty()) {
                ExportClasses(sFile, classes, exportType);
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

ImportType importType = ImportType::XML;
static int importState = 0;

bool PreImportPopup()
{
    if (classes.empty()) {
        return false;
    }
    ImGui::OpenPopup("Import File");
    if (ImGui::BeginPopupModal("Import File")) {
        if (ImGui::RadioButton("XML format", &exportState, 1)) {
            exportState = 1;
            importType = ImportType::XML;
        };
        if (ImGui::Button("Open..")) {
            std::string sFile = OpenDialog(importType, DxWindow);
            if (!sFile.empty()) {
                ImportClasses(sFile, importType, classes);
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

void InstanceTool()
{
    if (ImGui::Begin("Scanner Tool", 0, flags)) {
        if (ImGui::Button("Find Code References")) {
            codeReferences.clear();
            codeReferences = FindCodeReferences(targetSectionInfo->TEXT.base, targetSectionInfo->TEXT.size, (uintptr_t)currentClass->VTable ^ 0xDEADBEEF);
        }
        ImGui::SameLine();
        if (ImGui::Button("Find Instances") && currentClass) {
            instances.clear();
            instances = FindAllInstances((uintptr_t)currentClass->VTable);
        }
        if (instances.size() != 0) {
            ImGui::Text("found %d instances in memory", instances.size());
            for (unsigned int i = 0; i < instances.size(); i++) {
            bool badVtable = false;
                if (currentClass) {
                    if (!IsBadReadPointer((void*)instances[i])) {
                        if (*(uintptr_t*)instances[i] != ((uintptr_t)currentClass->VTable ^ 0xDEADBEEF))
                        {
                            ImGui::TextColored({ 255,0,0,1 }, POINTER_FMTSTRING, instances[i]);
                            if (ImGui::IsItemClicked()) {
                                char buffer[256] = { 0 };
                                sprintf_s(buffer, POINTER_FMTSTRING, instances[i]);
                                ImGui::SetClipboardText(buffer);
                            }
                            badVtable = true;
                        }
                    }
                    else {
                        ImGui::TextColored({ 255,0,0,1 }, POINTER_FMTSTRING, instances[i]);
                        if (ImGui::IsItemClicked()) {
                            char buffer[256] = { 0 };
                            sprintf_s(buffer, POINTER_FMTSTRING, instances[i]);
                            ImGui::SetClipboardText(buffer);
                        }
                        badVtable = true;
                    }
                }
                if (!badVtable) {
                    ImGui::TextColored({ 0,255,0,1 }, POINTER_FMTSTRING, instances[i]);
                    if (ImGui::IsItemClicked()) {
                        char buffer[256] = { 0 };
                        sprintf_s(buffer, POINTER_FMTSTRING, instances[i]);
                        ImGui::SetClipboardText(buffer);
                    }
                }
            }
        }
        if (codeReferences.size() != 0) {
            ImGui::Text("found %d vtable references in code", codeReferences.size());
            for (unsigned int i = 0; i < codeReferences.size(); i++) {
                ImGui::TextColored({ 0,255,225,1 }, POINTER_FMTSTRING, codeReferences[i]);
                if (ImGui::IsItemClicked()) {
                    char buffer[256] = { 0 };
                    sprintf_s(buffer, POINTER_FMTSTRING, codeReferences[i]);
                    ImGui::SetClipboardText(buffer);
                }
            }
        }
        ImGui::SetWindowPos(ImVec2{ 1500,0 });
        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::End();
    }
    
}


// doesn't support multiple instances yet, if it will idk
void AutoStructureDissect()
{
    auto size = currentClass->size;
    if (size == 0) return;
    if (instances.size() == 0) return;

    currentClass->members.clear();
    unsigned int i = 0;
    size_t offset = 0;
    while (1)
    {
        if (offset >= size) break;

        auto address = instances[i] + offset;
        size_t member_size = 0;
        MemberType type = AutoGuessType(address);

        switch (type)
        {
        case MemberType::type_boolean :
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
        member->baseAddress = instances[i];
        member->offset = offset;
        member->type = type;
        member->size = member_size;
        //if (type == MemberType::type_pointer) {
        //    auto ptr = member->baseAddress + member->offset;
        //    ptr = *(uintptr_t*)ptr;
        //    if (!IsBadReadPointer((void*)ptr))
        //    {
        //        if (IsValidSlow((void*)ptr, targetSectionInfo)) {
        //            ClassMeta meta = ClassMeta(ptr, targetSectionInfo);
        //            member->name = "vtable of" + meta.className;
        //        }
        //        else
        //        {
        //            auto ptr2 = *(uintptr_t*)ptr;
        //            if (!IsBadReadPointerAligned((void*)ptr2)) {
        //                if (IsValidSlow((void*)ptr2, targetSectionInfo)) {
        //                    ClassMeta meta = ClassMeta(ptr, targetSectionInfo);
        //                    member->name = "pointer to instance of" + meta.className;
        //                }
        //            }
        //        }
        //    }
        //    
        //}
        currentClass->members.push_back(member);
        offset += member_size;
    }
}

bool StructureDissectWindow()
{
    if (currentClass->members.size() == 0) return false;

    if (ImGui::Begin("Structure Dissect")){

        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::Text(POINTER_FMTSTRING, currentClass->members[0]->baseAddress);
        for (auto member : currentClass->members) {
            ImGui::Text("%d | %s | %s", member->offset, member->name, MemberType_str[member->type]);
        }
        ImGui::End();
    }
    return true;
}

MemberType AutoGuessType(uintptr_t address)
{
    // read every type from address
    float f = *(float*) address;
    double db = *(double*) address;
    unsigned long long qw = *(unsigned long long*) address;
    uintptr_t ptr = *(uintptr_t*) address;

    if (qw == 0)
        return MemberType::type_dword;

    if (!IsBadReadPointer( (void*) ptr))
        if(address % sizeof(uintptr_t) != 1)
            return MemberType::type_pointer;

    if (std::isnormal(f) && f > 0.0001 && f < 100000)
        return MemberType::type_float;

    if (std::isnormal(db) && db > 0.0001 && db < 100000)
        return MemberType::type_double;

    if (std::string((char*)address).size() > 5)
        return MemberType::type_string;

    return MemberType::type_dword;
}