#include "Shader.h"
#include "Utils.h"
#include "Debug.h"
#include "Window.h"
#include <d3dcompiler.h>
#include "GraphicDevice.h"
#include "Time.h"

Shader::Shader(const std::string& filePath)
{
#ifdef _DEBUG
	UINT flag = D3DCOMPILE_DEBUG;
#else
	UINT flag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	ID3DBlob* errorBlobVS;
	HRESULT result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vert", "vs_5_1", flag, 0, &m_vsBlob, &errorBlobVS);

	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			print("Vertex Shader: File not found");
		}
		else if (result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			print("Vertex Shader: Path not found");
		}
		else if (errorBlobVS)
		{
			print("Vertex Shader", (char*)errorBlobVS->GetBufferPointer());
			errorBlobVS->Release();
		}
		exit(1);
	}

	ID3DBlob* errorBlobPS;
	result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "frag", "ps_5_1", flag, 0, &m_psBlob, &errorBlobPS);

	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			print("Pixel Shader: File not found");
		}
		else if (result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			print("Pixel Shader: Path not found");
		}
		else if (errorBlobPS)
		{
			print("Pixel Shader", (char*)errorBlobPS->GetBufferPointer());
			errorBlobPS->Release();
		}
		exit(1);
	}
}

Shader::~Shader()
{
	m_psBlob->Release();
	m_vsBlob->Release();
}

Shader::SharedInput Shader::s_sharedInput = {};
ComPtr<ID3D12Resource> Shader::s_sharedConstantBufferResource = nullptr;
void* Shader::s_CBMap = nullptr;

void Shader::SetUpSharedResources(Descriptor::CPUHandle& cpuHandle)
{
	DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / Window::GetWidth();
	matrix.r[1].m128_f32[1] = 2.0f / Window::GetHeight();
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = -1.0f;

	s_sharedInput.projectionMatrix = matrix;

	D3D12_RESOURCE_DESC matrixDesc = Utils::ResourceDesc(Utils::AlignSize256(sizeof(SharedInput)));
	GraphicDevice::GetDevice()->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &matrixDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&s_sharedConstantBufferResource));

	s_sharedConstantBufferResource->Map(0, nullptr, &s_CBMap);
	memcpy(s_CBMap, &s_sharedInput, sizeof(SharedInput));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = s_sharedConstantBufferResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = s_sharedConstantBufferResource->GetDesc().Width;

	GraphicDevice::GetDevice()->CreateConstantBufferView(&cbvDesc, cpuHandle.Get());
	cpuHandle.Increment();
}

void Shader::UpdateSharedResources(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootConstantBufferView(0, s_sharedConstantBufferResource->GetGPUVirtualAddress());

	float time = Time::GetTime();
	s_sharedInput.time = float4(time / 20, time, time * 2, time * 3);
	s_sharedInput.deltaTime = Time::GetDeltaTime();
	memcpy(s_CBMap, &s_sharedInput, sizeof(SharedInput));
}
