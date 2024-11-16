#include "IndexBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

IndexBuffer::IndexBuffer(const std::vector<unsigned int>& indices):
	Buffer(sizeof(unsigned int) * indices.size()), m_view{}
{
	D3D12_RESOURCE_DESC resourceDesc = Utils::CreateResourceDesc(m_size);
	D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	GraphicDevice::GetDevice()->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));

	m_buffer->Map(0, nullptr, &m_map);
	memcpy(m_map, indices.data(), m_size);
	m_buffer->Unmap(0, nullptr);

	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.Format = DXGI_FORMAT_R32_UINT;
	m_view.SizeInBytes = m_size;
}

void IndexBuffer::Update(const std::vector<unsigned int>& indices)
{
	Buffer::Update((void*)indices.data());
}