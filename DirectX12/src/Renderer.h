#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h> 
#include "Utils.h"
#include <array>

using Microsoft::WRL::ComPtr;

class Renderer
{
private:
	static ComPtr<ID3D12Device> m_device;
	static ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	static ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	static ComPtr<ID3D12CommandQueue> m_cmdQueue;
	static ComPtr<ID3D12GraphicsCommandList> m_cmdList;
	static ComPtr<IDXGISwapChain4> m_swapChain;
	static ComPtr<ID3D12Fence> m_fence;
	static UINT64 m_fenceValue;
	static std::array<ComPtr<ID3D12Resource>, NUM_BACK_BUFFERS> m_backBufferResources;
	static HANDLE m_fenceHandle;
	static D3D12_VIEWPORT m_viewPort;
	static D3D12_RECT m_scissorRect;
	static D3D12_RESOURCE_BARRIER m_barrierRtv;
	static D3D12_RESOURCE_BARRIER m_barrierPresent;

	static constexpr float m_backgroundColor[4] = {0.01, 0.01, 0.01, 1};

public:
	static void Init();
	static void Update();
	static void ExecuteCommands(D3D12_RESOURCE_BARRIER* barrier = &m_barrierPresent);
	static void Render();
	static void WaitForFrame();
	static void Destroy();
	
	static ID3D12GraphicsCommandList* GetCommandList() { return m_cmdList.Get(); }
};