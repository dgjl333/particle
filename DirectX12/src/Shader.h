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
	enum class BlendType
	{
		Off, Alpha, Addictive
	};

	Shader(const std::string& filePath, BlendType type, bool hasGeometryShader = false);

	ID3DBlob* GetVS() const { return m_vsBlob.Get(); }
	ID3DBlob* GetPS() const { return m_psBlob.Get(); }
	ID3DBlob* GetGS() const { return m_gsBlob.Get(); }

	BlendType GetBlendType() const { return m_blendType; }

	static void ErrorCheck(const std::string& errorShader, const HRESULT& compileResult, ID3DBlob* errorBlob);

	static void SetUpSharedResources(Descriptor::CPUHandle& cpuHandle);
	static void UpdateSharedResources(ID3D12GraphicsCommandList* cmdList);

	static ID3DBlob* CompileComputeShader(const std::string& filePath);

	static D3D12_GPU_VIRTUAL_ADDRESS GetSharedConstantBufferGpuAddress() { return s_constantBuffer.get()->GetResource()->GetGPUVirtualAddress(); }

private:
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;
	ComPtr<ID3DBlob> m_gsBlob;

	BlendType m_blendType;

	static UINT s_compileFlag;

	struct SharedInput
	{
		float4 time;
		float deltaTime;

		DirectX::XMMATRIX projectionMatrix;
		float seed;
	};

	static std::unique_ptr<ConstantBuffer<SharedInput>> s_constantBuffer;
	static SharedInput s_sharedInput;
};