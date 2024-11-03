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
#include "RootSignature.h"
#include "Descriptor.h"

using namespace DirectX;
const float windowSize = 1;

int main()
{
	EnableDebug();
	
	Random::Init();
	GraphicDevice::Init();

	ID3D12Device* device = GraphicDevice::GetDevice();

	Window::Init(windowSize);

	GUI::Init();

	Renderer::Init();
	ID3D12GraphicsCommandList* cmdList = Renderer::GetCommandList();

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

	RootSignature rootSignature;
	rootSignature.Add(RangeType::CBV | RangeType::SRV);
	rootSignature.Add(RangeType::CBV);
	rootSignature.Add(RangeType::UAV);
	rootSignature.Add(RangeType::SRV);
	rootSignature.Serialize(&texture.GetSamplerDescription(), 1);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
	pipelineDesc.pRootSignature = rootSignature.Get();
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
	device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));

	Descriptor::Init(device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	Descriptor descriptor(rootSignature.GetNumDescriptors(), rootSignature.GetRootArgumentsOffsets());

	Descriptor::CPUHandle cpuHandle = descriptor.GetCPUHandle();
	Descriptor::GPUHandle gpuHandle = descriptor.GetGPUHandle();

	Shader::SetUpSharedResources(cpuHandle);

	float4 cccc = float4(0.5, 0.0, 0.0, 0.0);
	ID3D12Resource* constBuffer2 = nullptr;
	D3D12_RESOURCE_DESC float4Desc = Utils::ResourceDesc(Utils::AlignSize256(sizeof(float4)));
	device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &float4Desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer2));
	void* float4Map;
	constBuffer2->Map(0, nullptr, &float4Map);
	memcpy(float4Map, &cccc, sizeof(float4));
	constBuffer2->Unmap(0, nullptr);
	D3D12_CONSTANT_BUFFER_VIEW_DESC float4BufferView = {};
	float4BufferView.BufferLocation = constBuffer2->GetGPUVirtualAddress();
	float4BufferView.SizeInBytes = constBuffer2->GetDesc().Width;
	device->CreateConstantBufferView(&float4BufferView, cpuHandle.Get());
	cpuHandle.Increment();
	device->CreateShaderResourceView(texture.GetBuffer(), &texture.GetShaderResourceViewDescription(), cpuHandle.Get());

	std::vector<Vertex> vertices2 = {
	{{500.0f, 500.0f, 0.0f}, {0, 0}},
	{{1000.0f, 500.0f, 0.0f}, {1, 0}},
	{{1000.0f, 0.0f, 0.0f}, {1, 1}},
	{{500.0f, 0.0f, 0.0f}, {0, 1}}
	};

	VertexBuffer vb2(vertices2);
	Shader shader2("shader/Test.hlsl");
	D3D12_GRAPHICS_PIPELINE_STATE_DESC anotherPipelineDesc = {};
	anotherPipelineDesc.pRootSignature = rootSignature.Get();
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
	device->CreateGraphicsPipelineState(&anotherPipelineDesc, IID_PPV_ARGS(&anotherPipelineState));

	struct TestShaderInput
	{
		float4 color;
		XMMATRIX mat;
	};

	ID3D12Resource* testConstBuffer = nullptr;
	D3D12_RESOURCE_DESC bufferDescWithTime = Utils::ResourceDesc(Utils::AlignSize256(sizeof(TestShaderInput)));
	device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &bufferDescWithTime, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&testConstBuffer));

	XMMATRIX matrix = XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / Window::GetWidth();
	matrix.r[1].m128_f32[1] = 2.0f / Window::GetHeight();
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = -1.0f;
	TestShaderInput matrixWithTime = {};
	matrixWithTime.mat = matrix;
	matrixWithTime.color = float4(1.0f, 0.0f, 0.0f, 1.0f); 

	void* testMap = nullptr;
	testConstBuffer->Map(0, nullptr, &testMap);
	memcpy(testMap, &matrixWithTime, sizeof(TestShaderInput));
	testConstBuffer->Unmap(0, nullptr);

	cpuHandle.Increment();
	D3D12_CONSTANT_BUFFER_VIEW_DESC testCbvDesc = {};
	testCbvDesc.BufferLocation = testConstBuffer->GetGPUVirtualAddress();
	testCbvDesc.SizeInBytes = testConstBuffer->GetDesc().Width;

	device->CreateConstantBufferView(&testCbvDesc, cpuHandle.Get());

		struct Particle
	 {
 		float2 position;
 		float2 velocity;
	 };

	 const int particleCount = 100000;

	 std::vector<Particle> particles(particleCount);
	 for (int i = 0; i < particleCount; i++)
	 {
 		particles[i].velocity = { 0.0f, 0.0f };
		//particles[i].position = { Random::GetValue(), Random::GetValue()};
		particles[i].position = { std::lerp(0.0f, 1920.0f, (float)i / particleCount),800.0f};
	 }

	 D3D12_RESOURCE_DESC particleDesc = Utils::ResourceDesc(particleCount * sizeof(Particle), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	 D3D12_RESOURCE_DESC particleUploadDesc = Utils::ResourceDesc(particleCount * sizeof(Particle));

	 ID3D12Resource* particleBuffer;
	 device->CreateCommittedResource(&Utils::heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &particleDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&particleBuffer));

	 ID3D12Resource* particleUploadBuffer;
	 device->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &particleUploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&particleUploadBuffer));

	 void* particleMap;
	 particleUploadBuffer->Map(0, nullptr, &particleMap);
	 memcpy(particleMap, particles.data(), sizeof(Particle) * particleCount);
	 particleUploadBuffer->Unmap(0, nullptr);

	 cmdList->CopyResource(particleBuffer, particleUploadBuffer);

	 auto particleBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	 Renderer::ExecuteCommands(&particleBarrier);
	 Renderer::WaitForFrame();

	 particleUploadBuffer->Release();

	 cpuHandle.Increment();

	 D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	 uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	 uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	 uavDesc.Buffer.NumElements = particleCount;
	 uavDesc.Buffer.StructureByteStride = sizeof(Particle);

	 device->CreateUnorderedAccessView(particleBuffer, nullptr, &uavDesc, cpuHandle.Get());
	 ID3D10Blob* computeShaderBlob = Shader::CompileComputeShader("shader/ComputeShader.hlsl");

	 D3D12_COMPUTE_PIPELINE_STATE_DESC computePipeLineDesc = {};
	 computePipeLineDesc.CS.pShaderBytecode = computeShaderBlob->GetBufferPointer();
	 computePipeLineDesc.CS.BytecodeLength = computeShaderBlob->GetBufferSize();
	 computePipeLineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	 computePipeLineDesc.NodeMask = 0;
	 computePipeLineDesc.pRootSignature = rootSignature.Get();

	 ID3D12PipelineState* computePipeLineState = nullptr;
	 device->CreateComputePipelineState(&computePipeLineDesc, IID_PPV_ARGS(&computePipeLineState));

	 cpuHandle.Increment();
	 
	 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	 srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	 srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	 srvDesc.Buffer.FirstElement = 0;
	 srvDesc.Buffer.NumElements = particleCount;
	 srvDesc.Buffer.StructureByteStride = sizeof(Particle);
	 srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	 srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	 device->CreateShaderResourceView(particleBuffer, &srvDesc, cpuHandle.Get());

	 Shader particleShader("shader/Particle.hlsl");

	 D3D12_GRAPHICS_PIPELINE_STATE_DESC particlePipelineDesc = {};
	 particlePipelineDesc.pRootSignature = rootSignature.Get();
	 particlePipelineDesc.VS.pShaderBytecode = particleShader.GetVS()->GetBufferPointer();
	 particlePipelineDesc.VS.BytecodeLength = particleShader.GetVS()->GetBufferSize();
	 particlePipelineDesc.PS.pShaderBytecode = particleShader.GetPS()->GetBufferPointer();
	 particlePipelineDesc.PS.BytecodeLength = particleShader.GetPS()->GetBufferSize();
	 particlePipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	 particlePipelineDesc.RasterizerState = pipelineDesc.RasterizerState;
	 particlePipelineDesc.BlendState = pipelineDesc.BlendState;
	 particlePipelineDesc.InputLayout = { nullptr, 0 };
	 particlePipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	 particlePipelineDesc.NumRenderTargets = 1;
	 particlePipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	 particlePipelineDesc.SampleDesc.Count = 1;
	 particlePipelineDesc.SampleDesc.Quality = 0;

	 ID3D12PipelineState* particlePipelineState = nullptr;
	 device->CreateGraphicsPipelineState(&particlePipelineDesc, IID_PPV_ARGS(&particlePipelineState));

	Time::Init();

	while (Window::Update())
	{
		Time::Update();
		GUI::Update();
		Renderer::Update();
	
		cmdList->SetGraphicsRootSignature(rootSignature.Get());

		Shader::UpdateSharedResources(cmdList);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmdList->SetPipelineState(pipelineState);
		cmdList->IASetVertexBuffers(0, 1, &vb.GetView());
		cmdList->IASetIndexBuffer(&ib.GetView());

		cmdList->SetDescriptorHeaps(1, &descriptor.GetHeap());
		gpuHandle.ResetToGraphicsRootDescriptorTableStart();

		cmdList->SetGraphicsRootDescriptorTable(1, gpuHandle.Get());

		cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

		cmdList->SetPipelineState(anotherPipelineState);
		gpuHandle.Increment();

		cmdList->SetGraphicsRootDescriptorTable(2, gpuHandle.Get());
		cmdList->IASetVertexBuffers(0, 1, &vb2.GetView());

		cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

		gpuHandle.Increment();
		cmdList->SetComputeRootSignature(rootSignature.Get());
		cmdList->SetPipelineState(computePipeLineState);
		cmdList->SetComputeRootConstantBufferView(0, Shader::GetSharedConstantBufferGpuAddress());
		cmdList->SetComputeRootDescriptorTable(3, gpuHandle.Get());

		auto preBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->ResourceBarrier(1, &preBarrier);
		cmdList->Dispatch(ceil((float)particleCount / 32), 1, 1);

		gpuHandle.Increment();
		auto postBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		cmdList->ResourceBarrier(1, &postBarrier);
		cmdList->SetGraphicsRootDescriptorTable(4, gpuHandle.Get());
		cmdList->SetPipelineState(particlePipelineState);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmdList->DrawInstanced(particleCount, 1, 0, 0);


		GUI::Render(cmdList);

		Renderer::ExecuteCommands();
		Renderer::WaitForFrame();
		Renderer::Render();
	}
	Renderer::Destroy();
	GraphicDevice::Destroy();
	Window::Destroy();
}



