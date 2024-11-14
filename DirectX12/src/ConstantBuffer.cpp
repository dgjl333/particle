#include "ConstantBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"
#include "GUI.h"

ConstantBuffer::ConstantBuffer(void* buffer, UINT64 size):
	m_resource(nullptr), m_map(nullptr), m_updateRange(nullptr), m_desc({}), m_size(0)
{
	D3D12_RESOURCE_DESC desc = Utils::CreateResourceDesc(Utils::AlignSize256(size));
	D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	GraphicDevice::GetDevice()->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource));

	m_resource->Map(0, nullptr, &m_map);
	memcpy(m_map, buffer, size);
	m_resource->Unmap(0, nullptr);

	m_desc.BufferLocation = m_resource->GetGPUVirtualAddress();
	m_desc.SizeInBytes = m_resource->GetDesc().Width;

	m_size = size;
}

void ConstantBuffer::Map(D3D12_RANGE* updateRange)
{
	if (updateRange)
	{
		m_updateRange = std::make_unique<D3D12_RANGE>(*updateRange);
	}
	m_resource->Map(0, m_updateRange.get(), &m_map);
}

void ConstantBuffer::Unmap()
{
	m_resource->Unmap(0, m_updateRange.get());
}

void ConstantBuffer::Update(void* buffer)
{
	if (m_updateRange)
	{
		size_t size = m_updateRange.get()->End - m_updateRange.get()->Begin;
		memcpy(m_map, (char*)buffer + m_updateRange.get()->Begin, size);
	}
	else
	{
		memcpy(m_map, buffer, m_size);
	}
}
