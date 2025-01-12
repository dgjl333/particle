#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Buffer
{
public:
	Buffer(UINT64 size):
		m_map(nullptr), m_size(size), m_buffer(nullptr), m_updateRange(nullptr)
	{};
	virtual ~Buffer() = default;

	void Map(D3D12_RANGE* updateRange);
	void Unmap();

	inline ID3D12Resource* GetResource() const { return m_buffer.Get(); }

protected:
	void* m_map;
	UINT64 m_size;
	ComPtr<ID3D12Resource> m_buffer;
	std::unique_ptr<D3D12_RANGE> m_updateRange;

    void Update(void* buffer);
};