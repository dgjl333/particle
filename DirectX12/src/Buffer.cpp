#include "Buffer.h"

void Buffer::Map(D3D12_RANGE* updateRange)
{
	if (updateRange)
	{
		m_updateRange = std::make_unique<D3D12_RANGE>(*updateRange);
	}
	m_resource->Map(0, m_updateRange.get(), &m_map);
}

void Buffer::Unmap()
{
	m_resource->Unmap(0, m_updateRange.get());
}

void Buffer::Update(void* buffer)
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