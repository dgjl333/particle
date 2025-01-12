#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Utils.h"
#include <array>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Renderer
{
public:
	static void Init();
	static void Update();
	static void ExecuteCommands(D3D12_RESOURCE_BARRIER* barrier = &s_barrierPresent);
	static void Render();
	static void WaitForFrame();
	static void Destroy();
	
	static ID3D12GraphicsCommandList* GetCommandList() { return s_cmdList.Get(); }

private:
	static ComPtr<ID3D12Device> s_device;
	static ComPtr<ID3D12DescriptorHeap> s_rtvHeap;
	static ComPtr<ID3D12CommandAllocator> s_cmdAllocator;
	static ComPtr<ID3D12CommandQueue> s_cmdQueue;
	static ComPtr<ID3D12GraphicsCommandList> s_cmdList;
	static ComPtr<IDXGISwapChain4> s_swapChain;
	static ComPtr<ID3D12Fence> s_fence;
	static UINT64 s_fenceValue;
	static std::array<ComPtr<ID3D12Resource>, NUM_BACK_BUFFERS> s_backBufferResources;
	static HANDLE s_fenceHandle;
	static D3D12_VIEWPORT s_viewPort;
	static D3D12_RECT s_scissorRect;
	static D3D12_RESOURCE_BARRIER s_barrierRtv;
	static D3D12_RESOURCE_BARRIER s_barrierPresent;

	static constexpr float s_backgroundColor[4] = { 0.01, 0.01, 0.01, 1 };
};