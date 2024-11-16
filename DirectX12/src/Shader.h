#pragma once
#include <string>
#include <d3d12.h>
#include <memory>
#include <wrl/client.h> 
#include "Descriptor.h"
#include "MathType.h"
#include "ConstantBuffer.h"

using Microsoft::WRL::ComPtr;

class Shader
{
public:
	Shader(const std::string& filePath, bool hasGeometryShader = false);
	~Shader();

	ID3DBlob* GetVS() { return m_vsBlob; }
	ID3DBlob* GetPS() { return m_psBlob; }
	ID3DBlob* GetGS() { return m_gsBlob; }

	static void ErrorCheck(const std::string& errorShader, const HRESULT& compileResult, ID3DBlob* errorBlob);

	static void SetUpSharedResources(Descriptor::CPUHandle& cpuHandle);
	static void UpdateSharedResources(ID3D12GraphicsCommandList* cmdList);

	static ID3DBlob* CompileComputeShader(const std::string& filePath);

	static D3D12_GPU_VIRTUAL_ADDRESS GetSharedConstantBufferGpuAddress() { return s_constantBuffer.get()->GetResource()->GetGPUVirtualAddress(); }

private:
	ID3DBlob* m_vsBlob;
	ID3DBlob* m_psBlob;
	ID3DBlob* m_gsBlob;

	static UINT s_compileFlag;

	struct SharedInput
	{
		float4 time;
		float deltaTime;

		DirectX::XMMATRIX projectionMatrix;
		float seed;
	};

	static std::unique_ptr<ConstantBuffer> s_constantBuffer;
	static SharedInput s_sharedInput;
};