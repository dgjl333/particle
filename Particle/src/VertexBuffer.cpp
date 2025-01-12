#include "VertexBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices) :
	Buffer(sizeof(Vertex) * vertices.size()), m_view({})
{
	D3D12_RESOURCE_DESC resourceDesc = Utils::ResourceDesc(m_size);
	D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	GraphicDevice::GetDevice()->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));
	
	m_buffer->Map(0, nullptr, &m_map);
	memcpy(m_map, vertices.data(), m_size);
	m_buffer->Unmap(0, nullptr);

	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.SizeInBytes = m_size;
	m_view.StrideInBytes = sizeof(Vertex);
}

void VertexBuffer::Update(const std::vector<Vertex>& vertices)
{
	Buffer::Update((void*)vertices.data());
}
