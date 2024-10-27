#include "Texture.h"
#include "Windows.h"
#include "Utils.h"
#include "GraphicDevice.h"
#include "Renderer.h"

void Texture::Init()
{
	static bool isInitalized = false;
	if (!isInitalized)
	{
		 CoInitializeEx(NULL, COINIT_MULTITHREADED);
		 isInitalized = true;
	}
}

Texture::Texture(const std::string& filePath) :
	m_buffer(nullptr), m_srvDesc{}, m_samplerDesc{}, m_metaData{}
{
	Init();
	DirectX::ScratchImage scratchImg = {};

	LoadFromWICFile(Utils::GetWStringFromString(filePath).c_str(), DirectX::WIC_FLAGS_NONE, &m_metaData, scratchImg);
	const DirectX::Image* img = scratchImg.GetImage(0, 0, 0);

	m_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	m_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	m_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	m_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	m_samplerDesc.MinLOD = 0.0f;
	m_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	m_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	m_srvDesc.Format = m_metaData.format;
	m_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	m_srvDesc.Texture2D.MipLevels = 1;

	ID3D12Resource* uploadBuffer = nullptr;
	
	CreateResource(img, &uploadBuffer);

	Upload(img, uploadBuffer);

	D3D12_RESOURCE_BARRIER texBarrierDesc = Utils::ResourceBarrier(m_buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	Renderer::ExecuteCommands(&texBarrierDesc);
	Renderer::WaitForFrame();
	uploadBuffer->Release();
}

void Texture::CreateResource(const DirectX::Image* img, ID3D12Resource** uploadBuffer)
{
	UINT64 width = Utils::AlignSize256(img->rowPitch) * img->height;
	D3D12_RESOURCE_DESC uploadResourceDesc = Utils::ResourceDesc(width);
	GraphicDevice::GetDevice()->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(uploadBuffer));

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = m_metaData.width;
	textureDesc.Height = m_metaData.height;
	textureDesc.DepthOrArraySize = m_metaData.arraySize;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MipLevels = m_metaData.mipLevels;
	textureDesc.Dimension = (D3D12_RESOURCE_DIMENSION)m_metaData.dimension;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC uploadDesc = {};
	uploadDesc.Format = m_metaData.format;
	uploadDesc.Width = m_metaData.width;
	uploadDesc.Height = m_metaData.height;
	uploadDesc.DepthOrArraySize = m_metaData.arraySize;
	uploadDesc.MipLevels = m_metaData.mipLevels;
	uploadDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(m_metaData.dimension);
	uploadDesc.SampleDesc.Count = 1;
	uploadDesc.SampleDesc.Quality = 0;
	uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	GraphicDevice::GetDevice()->CreateCommittedResource(&texHeapProp, D3D12_HEAP_FLAG_NONE, &uploadDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_buffer));
}

void Texture::Upload(const DirectX::Image* img, ID3D12Resource* uploadBuffer)
{
	uint8_t* imageMap = nullptr;
	uploadBuffer->Map(0, nullptr, (void**)&imageMap);
	auto rowPitch = Utils::AlignSize256(img->rowPitch);
	auto srcAddress = img->pixels;
	for (int i = 0; i < img->height; i++)
	{
		std::copy_n(srcAddress, rowPitch, imageMap);
		srcAddress += img->rowPitch;
		imageMap += rowPitch;
	}
	uploadBuffer->Unmap(0, nullptr);

	D3D12_TEXTURE_COPY_LOCATION texCopyLocation = {};
	texCopyLocation.pResource = uploadBuffer;
	texCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	texCopyLocation.PlacedFootprint.Offset = 0;
	texCopyLocation.PlacedFootprint.Footprint.Width = m_metaData.width;
	texCopyLocation.PlacedFootprint.Footprint.Height = m_metaData.height;
	texCopyLocation.PlacedFootprint.Footprint.Depth = m_metaData.depth;
	texCopyLocation.PlacedFootprint.Footprint.RowPitch = Utils::AlignSize256(img->rowPitch);
	texCopyLocation.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION texDestLocation = {};
	texDestLocation.pResource = m_buffer;
	texDestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	texDestLocation.SubresourceIndex = 0;

	Renderer::GetCommandList()->CopyTextureRegion(&texDestLocation, 0, 0, 0, &texCopyLocation, nullptr);
}

Texture::~Texture()
{
	m_buffer->Release();
}

