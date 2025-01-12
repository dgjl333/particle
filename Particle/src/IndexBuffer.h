#pragma once

#include "Buffer.h"
#include <vector>
#include "Vertex.h"

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const std::vector<unsigned int>& indices);
	void Update(const std::vector<unsigned int>& indices);

	const D3D12_INDEX_BUFFER_VIEW& GetView() const { return m_view; }

private:
	D3D12_INDEX_BUFFER_VIEW m_view;
};
