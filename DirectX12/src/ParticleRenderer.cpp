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
#include "ConstantBuffer.h"
#include "../shader/ParticleData.hlsli"

using namespace DirectX;

int main()
{
	EnableDebug();

	Random::Init();
	GraphicDevice::Init();

	ID3D12Device* device = GraphicDevice::GetDevice();

	Window::Init();

	GUI::Init();

	Renderer::Init();
	ID3D12GraphicsCommandList* cmdList = Renderer::GetCommandList();

	Texture texture("assets/texture/1.png");

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
	rootSignature.Add(RangeType::CBV | RangeType::UAV);
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

	ConstantBuffer constantBuffer((void*)&cccc, sizeof(cccc));
	device->CreateConstantBufferView(&constantBuffer.GetDesc(), cpuHandle.Get());

	device->CreateShaderResourceView(texture.GetBuffer(), &texture.GetShaderResourceViewDescription(), cpuHandle.Increment());

	struct Particle
	{
		float2 position;
		float2 velocity;
	};

	struct ParticleInput
	{
		float test;
	};

	ParticleInput particleInput = {};
	particleInput.test = 1;
	ConstantBuffer particleInputBuffer((void*)&particleInput, sizeof(ParticleInput));
	cpuHandle.Increment();
	device->CreateConstantBufferView(&particleInputBuffer.GetDesc(), cpuHandle.Get());

	int particleCount = (PARTICLE_COUNT / PARTICLE_NUMTHREADS) * PARTICLE_NUMTHREADS;

	std::vector<Particle> particles(particleCount);
	for (int i = 0; i < particleCount; i++)
	{
		particles[i].velocity = { Random::GetValue() * 100, Random::GetValue() * 100 };
		particles[i].position = { Random::Range(0.0f, (float)Window::GetWidth()), Random::Range(0.0f, (float)Window::GetHeight()) };
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

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = particleCount;
	uavDesc.Buffer.StructureByteStride = sizeof(Particle);

	device->CreateUnorderedAccessView(particleBuffer, nullptr, &uavDesc, cpuHandle.Increment());
	ID3D10Blob* computeShaderBlob = Shader::CompileComputeShader("shader/ComputeShader.hlsl");

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipeLineDesc = {};
	computePipeLineDesc.CS.pShaderBytecode = computeShaderBlob->GetBufferPointer();
	computePipeLineDesc.CS.BytecodeLength = computeShaderBlob->GetBufferSize();
	computePipeLineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	computePipeLineDesc.NodeMask = 0;
	computePipeLineDesc.pRootSignature = rootSignature.Get();

	ID3D12PipelineState* computePipeLineState = nullptr;
	device->CreateComputePipelineState(&computePipeLineDesc, IID_PPV_ARGS(&computePipeLineState));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = particleCount;
	srvDesc.Buffer.StructureByteStride = sizeof(Particle);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	device->CreateShaderResourceView(particleBuffer, &srvDesc, cpuHandle.Increment());

	Shader particleShader("shader/Particle.hlsl", true);

	D3D12_RENDER_TARGET_BLEND_DESC particleBlendDesc = {}; //addictive blending
	particleBlendDesc.BlendEnable = true;
	particleBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	particleBlendDesc.DestBlend = D3D12_BLEND_ONE;
	particleBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	particleBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	particleBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	particleBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	particleBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC particlePipelineDesc = {};
	particlePipelineDesc.pRootSignature = rootSignature.Get();
	particlePipelineDesc.VS.pShaderBytecode = particleShader.GetVS()->GetBufferPointer();
	particlePipelineDesc.VS.BytecodeLength = particleShader.GetVS()->GetBufferSize();
	particlePipelineDesc.PS.pShaderBytecode = particleShader.GetPS()->GetBufferPointer();
	particlePipelineDesc.PS.BytecodeLength = particleShader.GetPS()->GetBufferSize();
	particlePipelineDesc.GS.pShaderBytecode = particleShader.GetGS()->GetBufferPointer();
	particlePipelineDesc.GS.BytecodeLength = particleShader.GetGS()->GetBufferSize();
	particlePipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	particlePipelineDesc.RasterizerState = pipelineDesc.RasterizerState;
	particlePipelineDesc.BlendState = pipelineDesc.BlendState;
	particlePipelineDesc.BlendState.RenderTarget[0] = particleBlendDesc;
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


		cmdList->SetDescriptorHeaps(1, &descriptor.GetHeap());
		gpuHandle.ResetToGraphicsRootDescriptorTableStart();

		//cmdList->SetGraphicsRootDescriptorTable(1, gpuHandle.Get());
		//cmdList->IASetVertexBuffers(0, 1, &vb.GetView());
		//cmdList->IASetIndexBuffer(&ib.GetView());
		//cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);

		cmdList->SetComputeRootSignature(rootSignature.Get());
		cmdList->SetPipelineState(computePipeLineState);
		cmdList->SetComputeRootConstantBufferView(0, Shader::GetSharedConstantBufferGpuAddress());
		cmdList->SetComputeRootDescriptorTable(2, gpuHandle.Increment());

		auto preBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->ResourceBarrier(1, &preBarrier);
		cmdList->Dispatch(particleCount / PARTICLE_NUMTHREADS, 1, 1);

		auto postBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		cmdList->ResourceBarrier(1, &postBarrier);
		cmdList->SetGraphicsRootDescriptorTable(3, gpuHandle.Increment());
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



