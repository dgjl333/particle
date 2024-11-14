#pragma once
#include <d3d12.h>
#include <wrl/client.h> 
#include <memory>

using Microsoft::WRL::ComPtr;

class ConstantBuffer
{
private:
	ComPtr<ID3D12Resource> m_resource;
	void* m_map;
	std::unique_ptr<D3D12_RANGE> m_updateRange;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_desc;
	UINT64 m_size;

public:
	ConstantBuffer(void* buffer, UINT64 size);

	const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetDesc() { return m_desc; }

	void Map(D3D12_RANGE* updateRange);
	void Unmap();
	void Update(void* buffer);

	inline ID3D12Resource* GetResource() { return m_resource.Get(); }
};