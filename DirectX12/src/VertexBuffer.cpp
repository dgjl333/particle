#include "VertexBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices) :
	m_buffer(nullptr), m_view{}
{
	UINT64 totalSize = vertices.size() * sizeof(Vertex);
	D3D12_RESOURCE_DESC resourceDesc = Utils::ResourceDesc(totalSize);
	GraphicDevice::GetDevice()->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));
	
	Vertex* vertMap = nullptr;
	m_buffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	m_buffer->Unmap(0, nullptr);

	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.SizeInBytes = totalSize;
	m_view.StrideInBytes = sizeof(Vertex);
}

VertexBuffer::~VertexBuffer()
{
	m_buffer->Release();
}