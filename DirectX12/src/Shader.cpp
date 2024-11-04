#include "Shader.h"
#include "Utils.h"
#include "Debug.h"
#include "Window.h"
#include <d3dcompiler.h>
#include "GraphicDevice.h"
#include "Time.h"

#ifdef _DEBUG
	UINT Shader::s_compileFlag = D3DCOMPILE_DEBUG;
#else
	UINT Shader::s_compileFlag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

Shader::Shader(const std::string& filePath, bool hasGeometryShader):
	m_vsBlob(nullptr), m_psBlob(nullptr), m_gsBlob(nullptr)
{
	ID3DBlob* errorBlob;

	HRESULT result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vert", "vs_5_1", s_compileFlag, 0, &m_vsBlob, &errorBlob);
	ErrorCheck("Vertex Shader", result, errorBlob);

	result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "frag", "ps_5_1", s_compileFlag, 0, &m_psBlob, &errorBlob);
	ErrorCheck("Pixel Shader", result, errorBlob);

	if (hasGeometryShader)
	{
		result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "geom", "gs_5_1", s_compileFlag, 0, &m_gsBlob, &errorBlob);
		ErrorCheck("Geometry Shader", result, errorBlob);
	}
}

void Shader::ErrorCheck(const std::string& errorShader, const HRESULT& compileResult, ID3DBlob* errorBlob)
{
	if (FAILED(compileResult))
	{
		if (compileResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			print(errorShader, "File not found");
		}
		else if (compileResult == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			print(errorShader, "Path not found");
		}
		else if (errorBlob)
		{
			print(errorShader, (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
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

ID3DBlob* Shader::CompileComputeShader(const std::string& filePath)
{
	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DCompileFromFile(Utils::GetWStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_1", s_compileFlag, 0, &shaderBlob, &errorBlob);
	ErrorCheck("Compute Shader", result, errorBlob);
	return shaderBlob;
}
