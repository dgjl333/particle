#pragma once
#include <d3d12.h>
#include <vector>
#include "CommonData.h"

class VertexBuffer
{
private:
	ID3D12Resource* m_buffer;
	D3D12_VERTEX_BUFFER_VIEW m_view;
public:
	VertexBuffer(const std::vector<Vertex>& vertices);
	~VertexBuffer();

	const D3D12_VERTEX_BUFFER_VIEW& GetView() { return m_view; }
};