#pragma once

#include <d3d12.h>
#include <vector>
#include "RootSignature.h"
#include "Shader.h"
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class PipelineState
{
public:
	static std::vector<D3D12_INPUT_ELEMENT_DESC> s_inputLayout;

	PipelineState(const RootSignature& rootSignature, const Shader& shader, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType, const std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayout = &s_inputLayout);

	inline ID3D12PipelineState* Get() const { return m_state.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_state;

	D3D12_RENDER_TARGET_BLEND_DESC GetBlendDesc(Shader::BlendType type);
};