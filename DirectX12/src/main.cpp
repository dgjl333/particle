#include "Debug.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include "CommonData.h"
#include "Utils.h"
#include "Shader.h"
#include "Window.h"
#include "GraphicDevice.h"
#include "Utils.h"
#include "GUI.h"
#include "Renderer.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace DirectX;
const float windowSize = 1;

int main()
{
	EnableDebug();
	HRESULT result;
	
	GraphicDevice::Init();

	ID3D12Device* device = GraphicDevice::GetDevice();

	Window::Init(windowSize);

	GUI::Init();

	Renderer::Init();
	ID3D12GraphicsCommandList* cmdList = Renderer::GetCommandList();

	Texture::Init();
	Texture texture("texture/1.png");

	D3D12_DESCRIPTOR_RANGE descRanges[2] = {};
	descRanges[0].NumDescriptors = 1;
	descRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRanges[0].BaseShaderRegister = 0;
	descRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descRanges[1].NumDescriptors = 1;
	descRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRanges[1].BaseShaderRegister = 0;
	descRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.DescriptorTable.pDescriptorRanges = descRanges;
	rootParam.DescriptorTable.NumDescriptorRanges = 2;

	std::vector<Vertex> vertices = {
		{{0.0f, 500.0f, 0.0f}, {0, 0}},
		{{500.0f, 500.0f, 0.0f}, {1, 0}},
		{{500.0f, 0.0f, 0.0f}, {1, 1}},
		{{0.0f, 0.0f, 0.0f}, {0, 1}}
	};

	std::vector<unsigned int> indices = {
		0,1,2,
		0,2,3
	};
	
	VertexBuffer vb(vertices);
	IndexBuffer ib(indices);

	Shader shader("shader/Basic.hlsl");

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
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pStaticSamplers = &texture.GetSamplerDescription();
	rootSignatureDesc.NumStaticSamplers = 1;
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
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleDesc.Quality = 0;

	ID3D12PipelineState* pipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));

	ID3D12DescriptorHeap* srvDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC  srvHeapDesc = {};
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.NumDescriptors = 2;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescHeap));

	XMMATRIX matrix = XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / Window::GetWidth();
	matrix.r[1].m128_f32[1] = 2.0f / Window::GetHeight();
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = -1.0f;

	ID3D12Resource* constBuffer = nullptr;
	D3D12_RESOURCE_DESC matrixDesc = Utils::ResourceDesc(Utils::AlignSize256(sizeof(matrix)));
	result = device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &matrixDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer));

	XMMATRIX* matrixMap;
	result = constBuffer->Map(0, nullptr, (void**)&matrixMap);
	*matrixMap = matrix;
	constBuffer->Unmap(0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapHanlde = srvDescHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateShaderResourceView(texture.GetBuffer(), &texture.GetShaderResourceViewDescription(), srvHeapHanlde);

	srvHeapHanlde.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constBuffer->GetDesc().Width;

	device->CreateConstantBufferView(&cbvDesc, srvHeapHanlde);

	while (Window::Update())
	{
		GUI::Update();
		Renderer::Update();

		cmdList->SetPipelineState(pipelineState);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, &vb.GetView());
		cmdList->IASetIndexBuffer(&ib.GetView());

		cmdList->SetGraphicsRootSignature(rootsignature);
		cmdList->SetDescriptorHeaps(1, &srvDescHeap);
		cmdList->SetGraphicsRootDescriptorTable(0, srvDescHeap->GetGPUDescriptorHandleForHeapStart());

		cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

		GUI::Render(cmdList);

		Renderer::ExecuteCommands();
		Renderer::WaitForFrame();
		Renderer::Render();
	}

	Renderer::Destroy();
	GraphicDevice::Destroy();
	Window::Destroy();
}
