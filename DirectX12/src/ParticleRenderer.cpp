#include "Debug.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include "MathType.h"
#include "Utils.h"
#include "Shader.h"
#include "Window.h"
#include "GraphicDevice.h"
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
#include "Input.h"
#include "PipelineState.h"
#include "imgui/imgui.h"
#include "../shader/ParticleData.hlsli"

using namespace DirectX;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
#ifdef _DEBUG
	EnableDebug();
	AllocConsole();
	SetConsoleTitleW(L"Debug Console");

	FILE* file;
	freopen_s(&file, "CONIN$", "r", stdin);
	freopen_s(&file, "CONOUT$", "w", stderr);
	freopen_s(&file, "CONOUT$", "w", stdout);
#endif

	Random::Init();
	GraphicDevice::Init();

	ID3D12Device* device = GraphicDevice::GetDevice();

	Window::Init();

	GUI::Init();

	Renderer::Init();
	ID3D12GraphicsCommandList* cmdList = Renderer::GetCommandList();

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

	RootSignature rootSignature;
	rootSignature.Add(RangeType::CBV | RangeType::UAV);
	rootSignature.Add(RangeType::SRV);
	rootSignature.Add(RangeType::CBV);
	rootSignature.Serialize(nullptr, 0);

	Descriptor::Init(device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	Descriptor descriptor(rootSignature.GetNumDescriptors(), rootSignature.GetRootArgumentsOffsets());

	Descriptor::CPUHandle cpuHandle = descriptor.GetCPUHandle();
	Descriptor::GPUHandle gpuHandle = descriptor.GetGPUHandle();

	Shader::SetUpSharedResources(cpuHandle);

	struct Particle
	{
		float2 position;
		float2 velocity;
	};

	struct ParticleInput
	{
		float2 mousePos;
		float strength;
	};

	ParticleInput particleInput = { {0,0}, 0 };
	ConstantBuffer particleInputBuffer(&particleInput);
	particleInputBuffer.Map(nullptr);
	device->CreateConstantBufferView(&particleInputBuffer.GetView(), cpuHandle.Get());

	int particleCount = (PARTICLE_COUNT / PARTICLE_NUMTHREADS) * PARTICLE_NUMTHREADS;

	std::vector<Particle> particles(particleCount);
	for (int i = 0; i < particleCount; i++)
	{
		particles[i].velocity = { Random::GetValue() * 100, Random::GetValue() * 100 };
		particles[i].position = { Random::Range(0.0f, (float)Window::GetWidth()), Random::Range(0.0f, (float)Window::GetHeight()) };
	}

	D3D12_RESOURCE_DESC particleDesc = Utils::ResourceDesc(particleCount * sizeof(Particle), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_RESOURCE_DESC particleUploadDesc = Utils::ResourceDesc(particleCount * sizeof(Particle));

	D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_HEAP_PROPERTIES defaultProp = Utils::HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ID3D12Resource* particleBuffer;
	device->CreateCommittedResource(&defaultProp, D3D12_HEAP_FLAG_NONE, &particleDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&particleBuffer));

	ID3D12Resource* particleUploadBuffer;
	device->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &particleUploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&particleUploadBuffer));

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

	Shader particleShader("shader/Particle.hlsl", Shader::BlendType::Addictive, true);

	PipelineState particleState(rootSignature, particleShader, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, nullptr);

	VertexBuffer vb(vertices);
	vb.Map(nullptr);
	IndexBuffer ib(indices);

	Shader shader("shader/Mouse.hlsl", Shader::BlendType::Alpha);
	PipelineState state(rootSignature, shader, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	struct MouseEffect
	{
		float flash;
	};
	MouseEffect effect = {};

	ConstantBuffer mouseEffectBuffer(&effect);
	mouseEffectBuffer.Map(nullptr);
	device->CreateConstantBufferView(&mouseEffectBuffer.GetView(), cpuHandle.Increment());

	Time::Init();

	while (Window::Update())
	{
		Time::Update();
		GUI::Update();
		Renderer::Update();

		cmdList->SetGraphicsRootSignature(rootSignature.Get());

		Shader::UpdateSharedResources(cmdList);

		cmdList->SetDescriptorHeaps(1, descriptor.GetHeapAddress());
		gpuHandle.ResetToGraphicsRootDescriptorTableStart();

		ImGui::Begin("Menu");


		ImGui::End();

		int tableIndex = 1;
		float2 mousePos = Input::GetMousePosition();
		if (Input::GetMouseButton(MouseButton::LEFT) || Input::GetMouseButton(MouseButton::RIGHT))
		{
			particleInput.mousePos = mousePos;
			effect.flash = 1;
		}
		else
		{
			effect.flash = 0;
		}
		mouseEffectBuffer.Update(&effect);

		static MouseButton activeButton;
		if (Input::GetMouseButtonDown(MouseButton::LEFT))
		{
			activeButton = MouseButton::LEFT;
			particleInput.strength = 1;
		}
		if (activeButton == MouseButton::LEFT && Input::GetMouseButtonUp(MouseButton::LEFT))
		{
			particleInput.strength = 0;
		}
		if (Input::GetMouseButtonDown(MouseButton::RIGHT))
		{
			activeButton = MouseButton::RIGHT;
			particleInput.strength = -2;
		}
		if (activeButton == MouseButton::RIGHT && Input::GetMouseButtonUp(MouseButton::RIGHT))
		{
			particleInput.strength = 0;
		}

		particleInputBuffer.Update(&particleInput);

		cmdList->SetComputeRootSignature(rootSignature.Get());
		cmdList->SetPipelineState(computePipeLineState);
		cmdList->SetComputeRootConstantBufferView(0, Shader::GetSharedConstantBufferGpuAddress());
		cmdList->SetComputeRootDescriptorTable(tableIndex, gpuHandle.Get());

		auto preBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->ResourceBarrier(1, &preBarrier);
		cmdList->Dispatch(particleCount / PARTICLE_NUMTHREADS, 1, 1);

		auto postBarrier = Utils::ResourceBarrier(particleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		cmdList->ResourceBarrier(1, &postBarrier);
		cmdList->SetGraphicsRootDescriptorTable(++tableIndex, gpuHandle.Increment());
		cmdList->SetPipelineState(particleState.Get());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmdList->DrawInstanced(particleCount, 1, 0, 0);

		if (!GUI::IsCursorShown() && GUI::IsCursorInsideClient())
		{
			const int size = 50;
			vertices[0].position = float3(mousePos.x - size, mousePos.y - size, 0);
			vertices[1].position = float3(mousePos.x + size, mousePos.y - size, 0);
			vertices[2].position = float3(mousePos.x + size, mousePos.y + size, 0);
			vertices[3].position = float3(mousePos.x - size, mousePos.y + size, 0);
			vb.Update(vertices);

			cmdList->SetPipelineState(state.Get());
			cmdList->SetGraphicsRootDescriptorTable(++tableIndex, gpuHandle.Increment());
			cmdList->IASetVertexBuffers(0, 1, &vb.GetView());
			cmdList->IASetIndexBuffer(&ib.GetView());
			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
		}

		GUI::Render(cmdList);

		Renderer::ExecuteCommands();
		Renderer::WaitForFrame();
		Renderer::Render();
		Input::ClearStates();
	}
	Renderer::Destroy();
	Window::Destroy();
	return 0;
}



