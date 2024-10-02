#include "Debug.h"
#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>

int windowHeight;
int windowWidth;
float windowSize = 0.5;

struct url
{

};
D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};


LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{
	D3D_FEATURE_LEVEL featureLevel;
	ID3D12Device* device = nullptr;
	IDXGIFactory6* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;

	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	std::vector<IDXGIAdapter*> allAdapters;
	IDXGIAdapter* adapter = nullptr;
	for (size_t i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		allAdapters.push_back(adapter);
	}

	for (const auto& adpt : allAdapters)
	{
		DXGI_ADAPTER_DESC desc = {};
		adpt->GetDesc(&desc);
		std::wstring description = desc.Description;
		if (description.find(L"NVIDIA") != std::string::npos)
		{
			adapter = adpt;
			break;
		}
	}

	for (D3D_FEATURE_LEVEL level : levels)
	{
		if (D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device)) == S_OK)
		{
			featureLevel = level;
			break;
		}
	}

	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("DX12");
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);
	RECT rect = { 0, 0, windowWidth, windowHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	

	HWND hwnd = CreateWindow(
		w.lpszClassName,
		_T("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* cmdList = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ID3D12DescriptorHeap* rtvHeaps = nullptr;

	swapchainDesc.Width = windowWidth;
	swapchainDesc.Height = windowHeight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);
	result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	result = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdAllocator));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapChain->GetDesc(&swcDesc);
	std::vector<ID3D12Resource*> backBuffers(swcDesc.BufferCount);
	for (size_t i = 0; i < swcDesc.BufferCount; i++)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(backBuffers[i], nullptr, handle);
	}

	result = cmdAllocator->Reset();
	unsigned int backBuferIndex = swapChain->GetCurrentBackBufferIndex();
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += backBuferIndex * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	float color[] = { 1, 0, 0, 1 };
	cmdList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	cmdList->Close();
	ID3D12CommandList* cmdLists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, cmdLists);
	cmdAllocator->Reset();
	cmdList->Reset(cmdAllocator, nullptr);
	swapChain->Present(1, 0);
 
	windowHeight = GetSystemMetrics(SM_CYSCREEN) * windowSize;
	windowWidth = GetSystemMetrics(SM_CXSCREEN) * windowSize;

	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	UnregisterClass(w.lpszClassName, w.hInstance);
}
