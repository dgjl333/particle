#include "Shader.h"
#include "Utils.h"
#include "Debug.h"
#include "Window.h"
#include <d3dcompiler.h>
#include "GraphicDevice.h"
#include "Time.h"
#include "Random.h"

#ifdef _DEBUG
	UINT Shader::s_compileFlag = D3DCOMPILE_DEBUG;
#else
	UINT Shader::s_compileFlag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

Shader::Shader(const std::string& filePath, BlendType type, bool hasGeometryShader):
	m_vsBlob(nullptr), m_psBlob(nullptr), m_gsBlob(nullptr), m_blendType(type)
{
	ID3DBlob* errorBlob;

	HRESULT result = D3DCompileFromFile(Utils::WStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vert", "vs_5_1", s_compileFlag, 0, &m_vsBlob, &errorBlob);
	ErrorCheck("Vertex Shader", result, errorBlob);

	result = D3DCompileFromFile(Utils::WStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "frag", "ps_5_1", s_compileFlag, 0, &m_psBlob, &errorBlob);
	ErrorCheck("Pixel Shader", result, errorBlob);

	if (hasGeometryShader)
	{
		result = D3DCompileFromFile(Utils::WStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "geom", "gs_5_1", s_compileFlag, 0, &m_gsBlob, &errorBlob);
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

Shader::SharedInput Shader::s_sharedInput = {};
std::unique_ptr<ConstantBuffer<Shader::SharedInput>> Shader::s_constantBuffer = nullptr;

void Shader::SetUpSharedResources(Descriptor::CPUHandle& cpuHandle)
{
	DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / Window::GetWidth();
	matrix.r[1].m128_f32[1] = 2.0f / Window::GetHeight();
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = -1.0f;

	s_sharedInput.projectionMatrix = matrix;
	s_sharedInput.seed = Random::GetValue();

	D3D12_RANGE range = {};
	range.Begin = 0;
	range.End = offsetof(SharedInput, projectionMatrix);

	s_constantBuffer = std::make_unique<ConstantBuffer<Shader::SharedInput>>(&s_sharedInput);
	s_constantBuffer.get()->Map(&range);
	GraphicDevice::GetDevice()->CreateConstantBufferView(&s_constantBuffer.get()->GetView(), cpuHandle.Get());
	cpuHandle.Increment();
}

void Shader::UpdateSharedResources(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootConstantBufferView(0, s_constantBuffer.get()->GetResource()->GetGPUVirtualAddress());

	float time = Time::GetTime();
	s_sharedInput.time = float4(time / 20, time, time * 2, time * 3);
	s_sharedInput.deltaTime = Time::GetDeltaTime();
	s_constantBuffer.get()->Update(&s_sharedInput);
}

ID3DBlob* Shader::CompileComputeShader(const std::string& filePath)
{
	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DCompileFromFile(Utils::WStringFromString(filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_1", s_compileFlag, 0, &shaderBlob, &errorBlob);
	ErrorCheck("Compute Shader", result, errorBlob);
	return shaderBlob;
}
