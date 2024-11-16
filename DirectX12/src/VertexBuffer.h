#pragma once
#include "Buffer.h"
#include "Vertex.h"
#include <vector>

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(const std::vector<Vertex>& vertices);
	void Update(const std::vector<Vertex>& vertices);

	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return m_view; }

private:
	D3D12_VERTEX_BUFFER_VIEW m_view;
};