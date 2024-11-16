#pragma once

#include <d3d12.h>
#include <wrl/client.h> 
#include <memory>

using Microsoft::WRL::ComPtr;

class Buffer
{
private:

	void* m_map;
	UINT64 m_size;
	ComPtr<ID3D12Resource> m_resource;
	std::unique_ptr<D3D12_RANGE> m_updateRange;

public:

	Buffer() {};

	void Map(D3D12_RANGE* updateRange);
	void Unmap();
	void Update(void* buffer);

	inline ID3D12Resource* GetResource() { return m_resource.Get(); }
};