#pragma once
#include <string>
#include <d3d12.h>
#include <wrl/client.h> 
#include "Descriptor.h"
#include "CommonData.h"

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

	static D3D12_GPU_VIRTUAL_ADDRESS GetSharedConstantBufferGpuAddress() { return s_sharedConstantBufferResource->GetGPUVirtualAddress(); }

private:
	ID3DBlob* m_vsBlob;
	ID3DBlob* m_psBlob;
	ID3DBlob* m_gsBlob;

	static UINT s_compileFlag;

	struct SharedInput
	{
		DirectX::XMMATRIX projectionMatrix;

		float4 time;
		float deltaTime;
	};

	static SharedInput s_sharedInput;
	static ComPtr<ID3D12Resource> s_sharedConstantBufferResource;
	static void* s_CBMap;
};