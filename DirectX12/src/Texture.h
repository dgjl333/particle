#pragma once
#include <d3d12.h>
#include <string>
#include "DirectXTex/DirectXTex.h"

class Texture
{
private:
	ID3D12Resource* m_buffer;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;
	DirectX::TexMetadata m_metaData;

	void Init();
	void CreateResource(const DirectX::Image* img, ID3D12Resource** uploadBuffer);
	void Upload(const DirectX::Image* img, ID3D12Resource* uploadBuffer);

public:
	Texture(const std::string& filePath);
	~Texture();

	ID3D12Resource* GetBuffer() { return m_buffer; }
	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetShaderResourceViewDescription() { return m_srvDesc; }
	const D3D12_STATIC_SAMPLER_DESC& GetSamplerDescription() { return m_samplerDesc; }
	const DirectX::TexMetadata& GetMetaData() { return m_metaData; }
};