#pragma once
#pragma once

#include <d3d12.h>
#include <vector>
#include "Vertex.h"

class IndexBuffer
{
private:
	ID3D12Resource* m_buffer;
	D3D12_INDEX_BUFFER_VIEW m_view;
public:
	IndexBuffer(const std::vector<unsigned int>& indices);
	~IndexBuffer();

	const D3D12_INDEX_BUFFER_VIEW& GetView() { return m_view; }
};
