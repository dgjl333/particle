#pragma once
#include "Buffer.h"
#include "Utils.h"
#include "GraphicDevice.h"

using Microsoft::WRL::ComPtr;

template<typename T>
class ConstantBuffer : public Buffer
{
public:
	ConstantBuffer(T* buffer):
		Buffer(sizeof(T)), m_view({})
	{
		D3D12_RESOURCE_DESC desc = Utils::ResourceDesc(Utils::AlignSize256(m_size));
		D3D12_HEAP_PROPERTIES uploadProp = Utils::HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		GraphicDevice::GetDevice()->CreateCommittedResource(&uploadProp, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer));

		m_buffer->Map(0, nullptr, &m_map);
		memcpy(m_map, buffer, m_size);
		m_buffer->Unmap(0, nullptr);

		m_view.BufferLocation = m_buffer->GetGPUVirtualAddress();
		m_view.SizeInBytes = m_buffer->GetDesc().Width;
	}

	void Update(T* buffer)
	{
		Buffer::Update((void*)buffer);
	}

	const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetView() const { return m_view; }

private:
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_view;
};