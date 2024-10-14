#include "IndexBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

IndexBuffer::IndexBuffer(const std::vector<unsigned int>& indices):
	m_buffer(nullptr), m_view{}
{
	UINT64 totalSize = indices.size() * sizeof(unsigned int);
	D3D12_RESOURCE_DESC resourceDesc = Utils::ResourceDesc(totalSize);
	GraphicDevice::GetDevice()->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));

	unsigned int* indexMap = nullptr;
	m_buffer->Map(0, nullptr, (void**)&indexMap);
	std::copy(indices.begin(), indices.end(), indexMap);
	m_buffer->Unmap(0, nullptr);

	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.Format = DXGI_FORMAT_R32_UINT;
	m_view.SizeInBytes = totalSize;
}

IndexBuffer::~IndexBuffer()
{
	m_buffer->Release();
}