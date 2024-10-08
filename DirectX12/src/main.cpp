#include "Debug.h"
#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <d3dcompiler.h>
#include "MathDG.h"
#include "Utils.h"
#include "Shader.h"
#include "Window.h"
#include "GraphicDevice.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

#define NUM_BACK_BUFFERS 2

const float windowSize = 0.5;

struct Vertex
{
	float3 position;
	float2 uv;
};

int main()
{
	EnableDebug();
	HRESULT result;

	GraphicDevice graphicDevice;
	ID3D12Device* device = graphicDevice.GetDevice();

	Window::OnStart(windowSize);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF(ProjectPath"font/ARIAL.TTF", 15);
	io.Fonts->Build();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Window::GetHWND());
	ID3D12DescriptorHeap* imguiHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC imguiDesc = {};
	imguiDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	imguiDesc.NumDescriptors = 1;
	imguiDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&imguiDesc, IID_PPV_ARGS(&imguiHeap));
	ImGui_ImplDX12_Init(device, NUM_BACK_BUFFERS, DXGI_FORMAT_R8G8B8A8_UNORM, imguiHeap, imguiHeap->GetCPUDescriptorHandleForHeapStart(), imguiHeap->GetGPUDescriptorHandleForHeapStart());
	
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));

	ID3D12GraphicsCommandList* cmdList = nullptr;
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ID3D12CommandQueue* cmdQueue = nullptr;
	result = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	IDXGISwapChain4* swapChain = nullptr;
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
	result = graphicDevice.GetFactory()->CreateSwapChainForHwnd(cmdQueue, Window::GetHWND(), &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);

	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapChain->GetDesc(&swcDesc);
	
	std::vector<ID3D12Resource*> backBuffers(swcDesc.BufferCount);
	for (size_t i = 0; i < swcDesc.BufferCount; i++)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(backBuffers[i], nullptr, handle);
	}

	ID3D12Fence* fence = nullptr;
	UINT64 fenceValue = 0;
	result = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	Vertex vertices[] =
	{
		{{-0.5f, -0.5f, 0.0f}, {0,0}} ,
		{{0.5f,  -0.5f, 0.0f} , {1,0}},
		{{0.5f, 0.5f, 0.0f} , {1,1}},
		{{-0.5f, 0.5f, 0.0f} , {0,1}},
	};

	unsigned int indices[] = {
		0,1,2,
		0,2,3
	};

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeof(vertices);
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuffer = nullptr;
	ID3D12Resource* indexBuffer = nullptr;

	result = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuffer));

	resourceDesc.Width = sizeof(indices);
	
	result = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));

	Vertex* vertMap = nullptr;
	result = vertBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuffer->Unmap(0, nullptr);

	unsigned int* indexMap = nullptr;
	indexBuffer->Map(0, nullptr, (void**)&indexMap);
	std::copy(std::begin(indices), std::end(indices), indexMap);
	indexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R32_UINT;
	ibView.SizeInBytes = sizeof(indices);

	Shader shader("shader/VertexShader.hlsl", "shader/PixelShader.hlsl");

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	ID3D12RootSignature* rootsignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3D10Blob* rootSigatureBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigatureBlob, &errorBlob);

	result = device->CreateRootSignature(0, rootSigatureBlob->GetBufferPointer(), rootSigatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	rootSigatureBlob->Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
	pipelineDesc.pRootSignature = rootsignature;
	pipelineDesc.VS.pShaderBytecode = shader.GetVS()->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = shader.GetVS()->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = shader.GetPS()->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = shader.GetPS()->GetBufferSize();
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.RasterizerState.MultisampleEnable = false;
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineDesc.RasterizerState.DepthClipEnable = true;
	pipelineDesc.BlendState.AlphaToCoverageEnable = false;
	pipelineDesc.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	pipelineDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);
	pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;

	ID3D12PipelineState* pipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = Window::GetWidth();
	viewport.Height = Window::GetHeight();
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	D3D12_RECT scissorRect = {};
	scissorRect.top = 0;
	scissorRect.left = 0;
	scissorRect.right = scissorRect.left + Window::GetWidth();
	scissorRect.bottom = scissorRect.top + Window::GetHeight();

	HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);

	float color[] = { 0.1, 0.1, 0.1, 1 };

	while (Window::OnUpdate())
	{
		
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always); 
		ImGui::SetNextWindowSize(ImVec2(100, 40), ImGuiCond_Always);
		ImGui::Begin("FPS Counter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
		ImGui::Text("%d", (int)ImGui::GetIO().Framerate);
		ImGui::End();

		unsigned int backBuferIndex = swapChain->GetCurrentBackBufferIndex();
		D3D12_RESOURCE_BARRIER barrierDesc = {};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.Transition.pResource = backBuffers[backBuferIndex];
		barrierDesc.Transition.Subresource = 0;
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		cmdList->ResourceBarrier(1, &barrierDesc);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += backBuferIndex * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
		cmdList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
		cmdList->SetPipelineState(pipelineState);
		cmdList->SetGraphicsRootSignature(rootsignature);
		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, &vbView);
		cmdList->IASetIndexBuffer(&ibView);
		cmdList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		cmdList->SetDescriptorHeaps(1, &imguiHeap);
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		cmdList->ResourceBarrier(1, &barrierDesc);

		cmdList->Close();

		ID3D12CommandList* cmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(1, cmdLists);

		cmdQueue->Signal(fence, ++fenceValue);

		if (fence->GetCompletedValue() != fenceValue)
		{
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		cmdAllocator->Reset();
		cmdList->Reset(cmdAllocator, nullptr);
		
		swapChain->Present(1, 0);
	}

	CloseHandle(fenceEvent);
	Window::OnDestroy();
}
