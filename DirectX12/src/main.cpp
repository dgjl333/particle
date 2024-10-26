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
#include "Random.h"
#include "Time.h"
#include "RootSignatureManager.h"

using namespace DirectX;
const float windowSize = 1;

struct SharedInput
{
	XMMATRIX projectionMatrix;

	float4 time;
	float deltaTime;
};

int main()
{
	EnableDebug();
	HRESULT result;
	
	Random::Init();
	GraphicDevice::Init();

	ID3D12Device* device = GraphicDevice::GetDevice();

	Window::Init(windowSize);

	GUI::Init();

	Renderer::Init();
	ID3D12GraphicsCommandList* cmdList = Renderer::GetCommandList();

	Texture::Init();
	Texture texture("texture/1.png");

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

	RootSignatureManager rootSignatureManager;
	rootSignatureManager.Add(RangeType::CBV | RangeType::SRV);
	rootSignatureManager.Add(RangeType::CBV);
	rootSignatureManager.Serialize(&texture.GetSamplerDescription(), 1);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
	pipelineDesc.pRootSignature = rootSignatureManager.GetSignature();
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
	srvHeapDesc.NumDescriptors = 4;  //num of cbv_srv_uav total amount
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescHeap));

	SharedInput sharedInput = {};
	XMMATRIX matrix = XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / Window::GetWidth();
	matrix.r[1].m128_f32[1] = 2.0f / Window::GetHeight();
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = -1.0f;

	sharedInput.projectionMatrix = matrix;

	ID3D12Resource* constBuffer = nullptr;
	D3D12_RESOURCE_DESC matrixDesc = Utils::ResourceDesc(Utils::AlignSize256(sizeof(SharedInput)));
	result = device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &matrixDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer));\

	XMMATRIX* sharedInputMap;
	result = constBuffer->Map(0, nullptr, (void**)&sharedInputMap);
	memcpy(sharedInputMap, &sharedInput, sizeof(SharedInput));
	//constBuffer->Unmap(0, nullptr);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constBuffer->GetDesc().Width;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHeapHanlde = srvDescHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateConstantBufferView(&cbvDesc, srvHeapHanlde);

	srvHeapHanlde.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	float4 cccc = float4(0.5, 0.0, 0.0, 0.0);
	ID3D12Resource* constBuffer2 = nullptr;
	D3D12_RESOURCE_DESC float4Desc = Utils::ResourceDesc(Utils::AlignSize256(sizeof(float4)));
	result = device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &float4Desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer2));
	void* float4Map;
	result = constBuffer2->Map(0, nullptr, &float4Map);
	memcpy(float4Map, &cccc, sizeof(float4));
	constBuffer2->Unmap(0, nullptr);
	D3D12_CONSTANT_BUFFER_VIEW_DESC float4BufferView = {};
	float4BufferView.BufferLocation = constBuffer2->GetGPUVirtualAddress();
	float4BufferView.SizeInBytes = constBuffer2->GetDesc().Width;
	device->CreateConstantBufferView(&float4BufferView, srvHeapHanlde);

	srvHeapHanlde.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	device->CreateShaderResourceView(texture.GetBuffer(), &texture.GetShaderResourceViewDescription(), srvHeapHanlde);


	std::vector<Vertex> vertices2 = {
	{{500.0f, 500.0f, 0.0f}, {0, 0}},
	{{1000.0f, 500.0f, 0.0f}, {1, 0}},
	{{1000.0f, 0.0f, 0.0f}, {1, 1}},
	{{500.0f, 0.0f, 0.0f}, {0, 1}}
	};

	VertexBuffer vb2(vertices2);
	Shader shader2("shader/Test.hlsl");
	D3D12_GRAPHICS_PIPELINE_STATE_DESC anotherPipelineDesc = {};
	anotherPipelineDesc.pRootSignature = rootSignatureManager.GetSignature();
	anotherPipelineDesc.VS.pShaderBytecode = shader2.GetVS()->GetBufferPointer();
	anotherPipelineDesc.VS.BytecodeLength = shader2.GetVS()->GetBufferSize();
	anotherPipelineDesc.PS.pShaderBytecode = shader2.GetPS()->GetBufferPointer();
	anotherPipelineDesc.PS.BytecodeLength = shader2.GetPS()->GetBufferSize();
	anotherPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	anotherPipelineDesc.RasterizerState = pipelineDesc.RasterizerState;
	anotherPipelineDesc.BlendState = pipelineDesc.BlendState;
	anotherPipelineDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	anotherPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	anotherPipelineDesc.NumRenderTargets = 1;
	anotherPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	anotherPipelineDesc.SampleDesc.Count = 1;
	anotherPipelineDesc.SampleDesc.Quality = 0;

	ID3D12PipelineState* anotherPipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&anotherPipelineDesc, IID_PPV_ARGS(&anotherPipelineState));

	struct TestShaderInput
	{
		float4 color;
		XMMATRIX mat;
	};

	ID3D12Resource* testConstBuffer = nullptr;
	D3D12_RESOURCE_DESC bufferDescWithTime = Utils::ResourceDesc(Utils::AlignSize256(sizeof(TestShaderInput)));
	result = device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &bufferDescWithTime, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&testConstBuffer));

	// Map the buffer and set its values
	TestShaderInput matrixWithTime = {};
	matrixWithTime.mat = matrix;
	matrixWithTime.color = float4(1.0f, 0.0f, 0.0f, 1.0f); // Set initial time values

	void* testMap = nullptr;
	result = testConstBuffer->Map(0, nullptr, &testMap);
	memcpy(testMap, &matrixWithTime, sizeof(TestShaderInput));
	testConstBuffer->Unmap(0, nullptr);

	srvHeapHanlde.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CONSTANT_BUFFER_VIEW_DESC testCbvDesc = {};
	testCbvDesc.BufferLocation = testConstBuffer->GetGPUVirtualAddress();
	testCbvDesc.SizeInBytes = testConstBuffer->GetDesc().Width;

	device->CreateConstantBufferView(&testCbvDesc, srvHeapHanlde);


	//struct Particle
	//{
	//	float2 position;
	//	float2 velocity;
	//};

	//const int particleCount = 100;

	//std::vector<Particle> particles(particleCount);
	//for (int i = 0; i < particleCount; i++)
	//{
	//	particles[i].velocity = { 0.0f, 0.0f };
	//	particles[i].position = { 15.0f * i,800.0f };
	//}

	//D3D12_RESOURCE_DESC particleDesc = Utils::ResourceDesc(particleCount * sizeof(Particle), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//D3D12_RESOURCE_DESC particleUploadDesc = Utils::ResourceDesc(particleCount * sizeof(Particle));

	//ID3D12Resource* particleBuffer;
	//result = device->CreateCommittedResource(&Utils::heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &particleDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&particleBuffer));

	//ID3D12Resource* particleUploadBuffer;
	//result = device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &particleUploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&particleUploadBuffer));

	//void* particleMap;
	//particleUploadBuffer->Map(0, nullptr, &particleMap);
	//memcpy(particleMap, particles.data(), sizeof(Particle)* particleCount);
	//particleUploadBuffer->Unmap(0, nullptr);

	//auto particleBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//cmdList->CopyResource(particleBuffer, particleUploadBuffer);

	//Renderer::ExecuteCommands(&particleBarrier);
	//Renderer::WaitForFrame();

	//particleUploadBuffer->Release();
	 
	//D3D12_ROOT_PARAMETER particleRootParameters[2];
	//particleRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	//particleRootParameters[0].Descriptor.ShaderRegister = 0;
	//particleRootParameters[0].Descriptor.RegisterSpace = 0;
	//particleRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//particleRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	//particleRootParameters[1].Descriptor.ShaderRegister = 0;
	//particleRootParameters[1].Descriptor.RegisterSpace = 0;
	//particleRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	//D3D12_ROOT_SIGNATURE_DESC particleRootSignatureDesc = {};
	//particleRootSignatureDesc.NumParameters = 2;
	//particleRootSignatureDesc.pParameters = particleRootParameters;
	//particleRootSignatureDesc.NumStaticSamplers = 0;
	//particleRootSignatureDesc.pStaticSamplers = nullptr;
	//particleRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//ID3D10Blob* particleRootSigatureBlob = nullptr;
	//D3D12SerializeRootSignature(&particleRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &particleRootSigatureBlob, &errorBlob);

	//ID3D12RootSignature* particleRootsignature = nullptr;
	//device->CreateRootSignature(0, particleRootSigatureBlob->GetBufferPointer(), particleRootSigatureBlob->GetBufferSize(), IID_PPV_ARGS(&particleRootsignature));

	//device->CreateUnorderedAccessView()

	Time::Init();

	while (Window::Update())
	{
		Time::Update();
		GUI::Update();
		Renderer::Update();
	
		cmdList->SetGraphicsRootSignature(rootSignatureManager.GetSignature());
		cmdList->SetPipelineState(pipelineState);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->IASetVertexBuffers(0, 1, &vb.GetView());
		cmdList->IASetIndexBuffer(&ib.GetView());

		cmdList->SetDescriptorHeaps(1, &srvDescHeap);
		auto gpuHandle = srvDescHeap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cmdList->SetGraphicsRootDescriptorTable(1, gpuHandle);
		cmdList->SetGraphicsRootConstantBufferView(0, constBuffer->GetGPUVirtualAddress());

		cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

		cmdList->SetPipelineState(anotherPipelineState);
		gpuHandle.ptr += 2 * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cmdList->SetGraphicsRootDescriptorTable(2, gpuHandle);
		cmdList->IASetVertexBuffers(0, 1, &vb2.GetView());

		float time = Time::GetTime();
		sharedInput.time = float4(time / 20 , time, time * 2, time * 3);
		memcpy(sharedInputMap, &sharedInput, sizeof(SharedInput));

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
