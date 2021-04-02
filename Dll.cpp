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
        RenderSceneDX12();
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

void MainGUI()
{
    if (ImGui::Begin("Class Dumper 2.0", 0, flags)) {
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
            VtableList.clear();
            classes.clear();
            SectionInfo* si = GetSectionInformation(targetModule);
            if (si) {
                bSectionInfoGood = true;
                VtableList = FindAllVTables(si);
                if (VtableList.size() != 0) {
                    bFoundVtables = true;
                    SortSymbols(VtableList);
                    std::string lastClassName = "";
                    for (uintptr_t vtable : VtableList)
                    {
                        auto cm = new ClassMeta(vtable, si);
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
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0, 155, 0)));
        ImGui::SameLine();
        if (ImGui::Button("Export")) {}
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

        if (ImGui::InputText(": Search classes", searchBuffer, 256)) {
            searchClasses.clear();
            bIsSearchActive = true;
            std::string searchString = searchBuffer;
            if (searchBuffer) {
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
        }

        ImGui::SetWindowPos(ImVec2{ 0,0 });
        ImGui::SetWindowSize(ImVec2{900,100});
        ImGui::End();
    }
}

void ClassViewer() {
    if (ImGui::Begin("ClassView", 0, flags)) {
        if (bSectionInfoGood && bFoundVtables) {
            if (bIsSearchActive) {
                for (auto cm : searchClasses) {
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
                        currentClass = cm;
                    }
                }
            }
            else
            {
                for (auto cm : classes) {
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
                        currentClass = cm;
                    }
                }
            }

        }
        ImGui::SetWindowPos(ImVec2{ 0,101 });
        ImGui::SetWindowSize(ImVec2{ 900,950 });
        ImGui::End();
    }
}

void ClassInspector()
{
    if (ImGui::Begin("Class Info"))
    {
        if (!currentClass) {
            ImGui::Text("Class not selected yet.");
        }
        else
        {

            ImGui::Text(POINTER_CLASSFMTSTR, (uintptr_t)currentClass->VTable, currentClass->className.c_str());

            if (ImGui::IsItemClicked()) {
                char buffer[1024] = { 0 };
                sprintf_s(buffer, POINTER_CLASSFMTSTR, (uintptr_t)currentClass->VTable, currentClass->className.c_str());
                
                ImGui::SetClipboardText(buffer);
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({255,0,0,1}, "Inheritance Tree");
            ImGui::Separator();
            unsigned int tabIndex = 0;
            unsigned int lastOffset = -1;
            if (currentClass->numBaseClasses >= 2) {
                for (unsigned int i = 0; i < currentClass->baseClassNames.size(); i++) {
                    if (lastOffset == currentClass->GetBaseClass(i + 1)->where.mdisp) {
                        tabIndex += 1;
                    }
                    else {
                        lastOffset = currentClass->GetBaseClass(i + 1)->where.mdisp;
                        tabIndex = 0;
                    }
                    std::string formatString;
                    for (unsigned int i = 0; i < tabIndex; i++) { formatString.append("\t"); }
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
            ImGui::Text("Number of virtual functions: %d", currentClass->VirtualFunctions.size());
        
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored({ 250,100,0,1 }, "Virtual Functions:");
            ImGui::Separator();
            {
                unsigned int index = 0;
                for (auto virtualFunction : currentClass->VirtualFunctions) {
                    ImGui::TextColored({ 250,0,130,1 }, VTABLE_FMTSTRING, index, virtualFunction, currentClass->VirtualFunctionNames[index].c_str());
                    index++;
                }
            }
        }


        ImGui::SetWindowPos(ImVec2{ 900,0 });
        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::End();
    }
}

void InstanceTool()
{
    if (ImGui::Begin("Instance Tool", 0, flags)) {
        if (ImGui::Button("Find Instances") && currentClass) {
            instances.clear();
            instances = FindAllInstances((uintptr_t)currentClass->VTable);
        }
        if (instances.size() != 0) {
            ImGui::Text("found %d instances", instances.size());
            for (unsigned int i = 0; i < instances.size(); i++) {
                ImGui::Text(POINTER_FMTSTRING, instances[i]);
                if (ImGui::IsItemClicked()) {
                    char buffer[256] = { 0 };
                    sprintf_s(buffer, POINTER_FMTSTRING, instances[i]);
                    ImGui::SetClipboardText(buffer);
                }
            }
        }
        ImGui::SetWindowPos(ImVec2{ 1500,0 });
        ImGui::SetWindowSize(ImVec2{ 600,1050 });
        ImGui::End();
    }
    
}

void ExportData()
{
    /*To Be Implemented!
    Idea: Will Have export settings, XML, json or plain text dump or other?*/
    /*Just so that people can parse it any way they like*/
}

void RenderSceneDX12()
{
    // Rendering
    ImGui::Render();

    FrameContext* frameCtx = dxApp.WaitForNextFrameResources();
    UINT backBufferIdx = dxApp.m_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = dxApp.m_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    dxApp.m_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);
    dxApp.m_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    const float backgroundColor[4] = { 0, 0, 0, 1.0 };
    dxApp.m_pd3dCommandList->ClearRenderTargetView(dxApp.m_mainRenderTargetDescriptor[backBufferIdx], backgroundColor, 0, NULL);
    dxApp.m_pd3dCommandList->OMSetRenderTargets(1, &dxApp.m_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
    dxApp.m_pd3dCommandList->SetDescriptorHeaps(1, &dxApp.m_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxApp.m_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    dxApp.m_pd3dCommandList->ResourceBarrier(1, &barrier);
    dxApp.m_pd3dCommandList->Close();

    dxApp.m_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&dxApp.m_pd3dCommandList);

    dxApp.m_pSwapChain->Present(1, 0); // Present with vsync
    //dxApp.m_pSwapChain->Present(0, 0); // Present without vsync

    UINT64 fenceValue = dxApp.m_fenceLastSignaledValue + 1;
    dxApp.m_pd3dCommandQueue->Signal(dxApp.m_fence, fenceValue);
    dxApp.m_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
}
