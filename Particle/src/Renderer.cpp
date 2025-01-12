#include "Renderer.h"
#include "Window.h"
#include "GraphicDevice.h"
#include <vector>

ComPtr<ID3D12Device> Renderer::s_device = nullptr;
ComPtr<ID3D12DescriptorHeap> Renderer::s_rtvHeap = nullptr;
ComPtr<ID3D12CommandAllocator> Renderer::s_cmdAllocator = nullptr;
ComPtr<ID3D12CommandQueue> Renderer::s_cmdQueue = nullptr;
ComPtr<ID3D12GraphicsCommandList> Renderer::s_cmdList = nullptr;
ComPtr<IDXGISwapChain4> Renderer::s_swapChain = nullptr;
ComPtr<ID3D12Fence> Renderer::s_fence = nullptr;
UINT64 Renderer::s_fenceValue = 0;
std::array<ComPtr<ID3D12Resource>, NUM_BACK_BUFFERS> Renderer::s_backBufferResources = {};
HANDLE Renderer::s_fenceHandle = nullptr;
D3D12_VIEWPORT Renderer::s_viewPort = {};
D3D12_RECT Renderer::s_scissorRect = {};
D3D12_RESOURCE_BARRIER Renderer::s_barrierRtv = {};
D3D12_RESOURCE_BARRIER Renderer::s_barrierPresent = {};

void Renderer::Init()
{
	ID3D12Device* device = GraphicDevice::GetDevice();

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&s_cmdAllocator));

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, s_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&s_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&s_cmdQueue));

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = Window::GetWidth();
	swapchainDesc.Height = Window::GetHeight();
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = NUM_BACK_BUFFERS;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	GraphicDevice::GetFactory()->CreateSwapChainForHwnd(s_cmdQueue.Get(), Window::GetHWND(), &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)s_swapChain.GetAddressOf());

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&s_rtvHeap));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	s_swapChain->GetDesc(&swcDesc);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (size_t i = 0; i < NUM_BACK_BUFFERS; i++)
	{
		s_swapChain->GetBuffer(i, IID_PPV_ARGS(&s_backBufferResources[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle = s_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(s_backBufferResources[i].Get(), &rtvDesc, handle);
	}

	device->CreateFence(s_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_fence));

	s_fenceHandle = CreateEvent(nullptr, false, false, nullptr);

	s_viewPort.Width = Window::GetWidth();
	s_viewPort.Height = Window::GetHeight();
	s_viewPort.TopLeftX = 0;
	s_viewPort.TopLeftY = 0;
	s_viewPort.MinDepth = 0;
	s_viewPort.MaxDepth = 1;

	s_scissorRect.top = 0;
	s_scissorRect.left = 0;
	s_scissorRect.right = s_scissorRect.left + Window::GetWidth();
	s_scissorRect.bottom = s_scissorRect.top + Window::GetHeight();

	s_barrierRtv = Utils::ResourceBarrier(nullptr, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	s_barrierPresent = Utils::ResourceBarrier(nullptr, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void Renderer::Update()
{
	s_cmdAllocator->Reset();
	s_cmdList->Reset(s_cmdAllocator.Get(), nullptr);

	unsigned int backBuferIndex = s_swapChain->GetCurrentBackBufferIndex();
	s_barrierRtv.Transition.pResource = s_backBufferResources[backBuferIndex].Get();
	s_barrierPresent.Transition.pResource = s_backBufferResources[backBuferIndex].Get();
	s_cmdList->ResourceBarrier(1, &s_barrierRtv);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = s_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += backBuferIndex * GraphicDevice::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	s_cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	s_cmdList->ClearRenderTargetView(rtvHandle, s_backgroundColor, 0, nullptr);
	s_cmdList->RSSetViewports(1, &s_viewPort);
	s_cmdList->RSSetScissorRects(1, &s_scissorRect);
}

void Renderer::ExecuteCommands(D3D12_RESOURCE_BARRIER* barrier)
{
	if (barrier)
	{
		s_cmdList->ResourceBarrier(1, barrier);
	}

	s_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { s_cmdList.Get()};
	s_cmdQueue->ExecuteCommandLists(1, cmdLists);
}

void Renderer::WaitForFrame()
{
	s_cmdQueue->Signal(s_fence.Get(), ++s_fenceValue);

	if (s_fence->GetCompletedValue() != s_fenceValue)
	{
		s_fence->SetEventOnCompletion(s_fenceValue, s_fenceHandle);
		WaitForSingleObject(s_fenceHandle, INFINITE);
	}
}

void Renderer::Render()
{
	s_swapChain->Present(1, 0);
}

void Renderer::Destroy()
{
	CloseHandle(s_fenceHandle);
}

