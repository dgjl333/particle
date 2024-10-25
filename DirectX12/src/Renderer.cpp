#include "Renderer.h"
#include "Window.h"
#include "GraphicDevice.h"
#include <vector>

ComPtr<ID3D12Device> Renderer::m_device = nullptr;
ComPtr<ID3D12DescriptorHeap> Renderer::m_rtvHeap = nullptr;
ComPtr<ID3D12CommandAllocator> Renderer::m_cmdAllocator = nullptr;
ComPtr<ID3D12CommandQueue> Renderer::m_cmdQueue = nullptr;
ComPtr<ID3D12GraphicsCommandList> Renderer::m_cmdList = nullptr;
ComPtr<IDXGISwapChain4> Renderer::m_swapChain = nullptr;
ComPtr<ID3D12Fence> Renderer::m_fence = nullptr;
UINT64 Renderer::m_fenceValue = 0;
std::array<ComPtr<ID3D12Resource>, NUM_BACK_BUFFERS> Renderer::m_backBufferResources = {};
HANDLE Renderer::m_fenceHandle = nullptr;
D3D12_VIEWPORT Renderer::m_viewPort = {};
D3D12_RECT Renderer::m_scissorRect = {};
D3D12_RESOURCE_BARRIER Renderer::m_barrierRtv = {};
D3D12_RESOURCE_BARRIER Renderer::m_barrierPresent = {};

void Renderer::Init()
{
	ID3D12Device* device = GraphicDevice::GetDevice();

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator));

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_cmdQueue));

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

	GraphicDevice::GetFactory()->CreateSwapChainForHwnd(m_cmdQueue.Get(), Window::GetHWND(), &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)m_swapChain.GetAddressOf());

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	m_swapChain->GetDesc(&swcDesc);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (size_t i = 0; i < NUM_BACK_BUFFERS; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferResources[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(m_backBufferResources[i].Get(), &rtvDesc, handle);
	}

	device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	m_fenceHandle = CreateEvent(nullptr, false, false, nullptr);

	m_viewPort.Width = Window::GetWidth();
	m_viewPort.Height = Window::GetHeight();
	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.MinDepth = 0;
	m_viewPort.MaxDepth = 1;

	m_scissorRect.top = 0;
	m_scissorRect.left = 0;
	m_scissorRect.right = m_scissorRect.left + Window::GetWidth();
	m_scissorRect.bottom = m_scissorRect.top + Window::GetHeight();

	m_barrierRtv = Utils::ResourceBarrier(nullptr, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_barrierPresent = Utils::ResourceBarrier(nullptr, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void Renderer::Update()
{
	unsigned int backBuferIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_barrierRtv.Transition.pResource = m_backBufferResources[backBuferIndex].Get();
	m_barrierPresent.Transition.pResource = m_backBufferResources[backBuferIndex].Get();
	m_cmdList->ResourceBarrier(1, &m_barrierRtv);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += backBuferIndex * GraphicDevice::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	m_cmdList->ClearRenderTargetView(rtvHandle, m_backgroundColor, 0, nullptr);
	m_cmdList->RSSetViewports(1, &m_viewPort);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);
}

void Renderer::ExecuteCommands(D3D12_RESOURCE_BARRIER* barrier)
{
	if (barrier)
	{
		m_cmdList->ResourceBarrier(1, barrier);
	}

	m_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { m_cmdList.Get()};
	m_cmdQueue->ExecuteCommandLists(1, cmdLists);
}

void Renderer::WaitForFrame()
{
	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceValue);

	if (m_fence->GetCompletedValue() != m_fenceValue)
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceHandle);
		WaitForSingleObject(m_fenceHandle, INFINITE);
	}
	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);
}

void Renderer::Render()
{
	m_swapChain->Present(1, 0);
}

void Renderer::Destroy()
{
	CloseHandle(m_fenceHandle);
}

