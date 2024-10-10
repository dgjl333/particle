#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Renderer
{
private:
	static ComPtr<ID3D12Device> m_device;
	static ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	static ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	static ComPtr<ID3D12CommandQueue> m_cmdQueue;
	static ComPtr<IDXGISwapChain4> m_swapChain;

public:
	static void Init();
	static void Update();
};