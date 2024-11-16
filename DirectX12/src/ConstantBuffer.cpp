#include "ConstantBuffer.h"
#include "Utils.h"
#include "GraphicDevice.h"
#include "GUI.h"

ConstantBuffer::ConstantBuffer(void* buffer, UINT64 size):
	Buffer(size), m_view({})
{
	D3D12_RESOURCE_DESC desc = Utils::CreateResourceDesc(Utils::AlignSize256(size));
	D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	GraphicDevice::GetDevice()->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));

	m_buffer->Map(0, nullptr, &m_map);
	memcpy(m_map, buffer, size);
	m_buffer->Unmap(0, nullptr);

	m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
	m_view.SizeInBytes = m_buffer->GetDesc().Width;
}

