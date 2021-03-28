#pragma once
#define IMGUI_USER_CONFIG "custom_config.h"

#include "dx12/imgui_impl_dx12.h"
#include "dx12/imgui_impl_win32.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

	struct FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		UINT64                  FenceValue;
	};

class ImGuiApp
{
public:
	static int const NUM_FRAMES_IN_FLIGHT = 3;
	static int const NUM_BACK_BUFFERS = 3;
	HWND m_hwnd;
	FrameContext m_frameContext[NUM_FRAMES_IN_FLIGHT];
	UINT m_frameIndex;
	ID3D12Device* m_pd3dDevice;
	ID3D12DescriptorHeap* m_pd3dRtvDescHeap;
	ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	ID3D12Fence* m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceLastSignaledValue;
	IDXGISwapChain3* m_pSwapChain;
	HANDLE m_hSwapChainWaitableObject;
	ID3D12Resource* m_mainRenderTargetResource[NUM_BACK_BUFFERS];
	D3D12_CPU_DESCRIPTOR_HANDLE  m_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	bool SetupBackend();
	void CreateFrame();
	void ResizeSwapChain(int width, int height);
	void ShutdownBackend();
};

