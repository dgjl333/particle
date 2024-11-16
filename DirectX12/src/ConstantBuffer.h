#pragma once
#include "Buffer.h"

using Microsoft::WRL::ComPtr;

class ConstantBuffer : public Buffer
{
public:
	ConstantBuffer(void* buffer, UINT64 size);

	const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetView() const { return m_view; }

private:
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_view;
};