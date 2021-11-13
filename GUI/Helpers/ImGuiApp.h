#pragma once
#define IMGUI_USER_CONFIG "custom_config.h"
#include "DX12/imgui_impl_win32.h"
#include "..\..\RenderConfig.h"
#ifdef USE_DX12
#include "DX12/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
static int const NUM_FRAMES_IN_FLIGHT = 3;
static int const NUM_BACK_BUFFERS = 3;
struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class ImGuiAppDX12
{
public:

	static HWND m_hwnd;
	static FrameContext m_frameContext[NUM_FRAMES_IN_FLIGHT];
	static UINT m_frameIndex;
	static ID3D12Device* m_pd3dDevice;
	static ID3D12DescriptorHeap* m_pd3dRtvDescHeap;
	static ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	static ID3D12CommandQueue* m_pd3dCommandQueue;
	static ID3D12GraphicsCommandList* m_pd3dCommandList;
	static ID3D12Fence* m_fence;
	static HANDLE m_fenceEvent;
	static UINT64 m_fenceLastSignaledValue;
	static IDXGISwapChain3* m_pSwapChain;
	static HANDLE m_hSwapChainWaitableObject;
	static ID3D12Resource* m_mainRenderTargetResource[NUM_BACK_BUFFERS];
	static D3D12_CPU_DESCRIPTOR_HANDLE  m_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];

	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void WaitForLastSubmittedFrame();
	static FrameContext* WaitForNextFrameResources();
	static void CreateRenderTarget();
	static void CleanupRenderTarget();
	static bool SetupBackend();
	static void CreateFrame();
	static void ResizeSwapChain(int width, int height);
	static void ShutdownBackend();
	static void RenderFrame();
};

#else
#include "DX11/imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class ImGuiAppDX11 {
public:
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static HWND m_hwnd;
	static ID3D11Device* g_pd3dDevice;
	static ID3D11DeviceContext* g_pd3dDeviceContext;
	static IDXGISwapChain* g_pSwapChain;
	static ID3D11RenderTargetView* g_mainRenderTargetView;

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void CreateRenderTarget();
	static void CleanupRenderTarget();
	static void SetupBackend();
	static void ShutdownBackend();
	void CreateFrame();
	void RenderFrame();
};

#endif