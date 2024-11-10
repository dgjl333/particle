#include "ConstantBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

ConstantBuffer::ConstantBuffer(void* buffer, UINT64 size):
	m_resource(nullptr), m_map(nullptr), m_updateRange({}), m_desc({})
{
	D3D12_RESOURCE_DESC desc = Utils::ResourceDesc(Utils::AlignSize256(size));
	GraphicDevice::GetDevice()->CreateCommittedResource(&Utils::heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource));

	m_resource->Map(0, nullptr, &m_map);
	memcpy(m_map, buffer, size);
	m_resource->Unmap(0, nullptr);

	m_desc.BufferLocation = m_resource->GetGPUVirtualAddress();
	m_desc.SizeInBytes = m_resource->GetDesc().Width;
}

void ConstantBuffer::Map(D3D12_RANGE updateRange)
{
	m_updateRange = updateRange;
	m_resource->Map(0, &m_updateRange, &m_map);
}

void ConstantBuffer::Unmap()
{
	m_resource->Unmap(0, &m_updateRange);
}

void ConstantBuffer::Update(void* buffer)
{
	size_t size = m_updateRange.End - m_updateRange.Begin;
	memcpy(m_map, (char*)buffer + m_updateRange.Begin, size);
}
