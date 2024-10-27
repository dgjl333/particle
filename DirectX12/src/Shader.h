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
	Shader(const std::string& filePath);
	~Shader();

	ID3DBlob* GetVS() { return m_vsBlob; }
	ID3DBlob* GetPS() { return m_psBlob; }

	static void SetUpSharedResources(Descriptor::CPUHandle& cpuHandle);
	static void UpdateSharedResources(ID3D12GraphicsCommandList* cmdList);

private:
	ID3DBlob* m_vsBlob;
	ID3DBlob* m_psBlob;

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