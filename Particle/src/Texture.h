#pragma once
#include <d3d12.h>
#include <string>
#include "DirectXTex/DirectXTex.h"
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Texture
{
public:
	Texture(const std::string& filePath);

	ID3D12Resource* GetBuffer() { return m_buffer.Get(); }
	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetShaderResourceViewDescription() const { return m_srvDesc; }
	const D3D12_STATIC_SAMPLER_DESC& GetSamplerDescription() const { return m_samplerDesc; }
	const DirectX::TexMetadata& GetMetaData() const { return m_metaData; }

private:
	ComPtr<ID3D12Resource> m_buffer;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
	DirectX::TexMetadata m_metaData;

	void Init();
	void CreateResource(const DirectX::Image* img, ID3D12Resource** uploadBuffer);
	void Upload(const DirectX::Image* img, ID3D12Resource* uploadBuffer);
};