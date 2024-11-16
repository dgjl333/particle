#include "PipelineState.h"
#include "GraphicDevice.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> PipelineState::s_inputLayout = {
	{
		"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
	},
	{
		"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
	},
};

PipelineState::PipelineState(const RootSignature& rootSignature, const Shader& shader, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType, const std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayout):
	m_state(nullptr)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = rootSignature.Get();
	desc.VS.pShaderBytecode = shader.GetVS()->GetBufferPointer();
	desc.VS.BytecodeLength = shader.GetVS()->GetBufferSize();
	desc.PS.pShaderBytecode = shader.GetPS()->GetBufferPointer();
	desc.PS.BytecodeLength = shader.GetPS()->GetBufferSize();
	ID3DBlob* gs = shader.GetGS();
	if (gs)
	{
		desc.GS.pShaderBytecode = gs->GetBufferPointer();
		desc.GS.BytecodeLength = gs->GetBufferSize();
	}

	desc.BlendState.RenderTarget[0] = GetBlendDesc(shader.GetBlendType());
	if (inputLayout)
	{
		desc.InputLayout.pInputElementDescs = inputLayout->data();
		desc.InputLayout.NumElements = inputLayout->size();
	}
	else
	{
		desc.InputLayout.pInputElementDescs = nullptr;
		desc.InputLayout.NumElements = 0;
	}

	desc.PrimitiveTopologyType = primitiveType;
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	desc.RasterizerState.MultisampleEnable = false;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.DepthClipEnable = true;
	desc.BlendState.AlphaToCoverageEnable = false;
	desc.BlendState.IndependentBlendEnable = false;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	GraphicDevice::GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_state));
}

D3D12_RENDER_TARGET_BLEND_DESC PipelineState::GetBlendDesc(Shader::BlendType type)
{
	D3D12_RENDER_TARGET_BLEND_DESC desc = {};
	desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	switch (type)
	{
	case Shader::BlendType::Off:
		desc.BlendEnable = false;
		break;
	case Shader::BlendType::Alpha:
		desc.BlendEnable = true;
		break;
	case Shader::BlendType::Addictive:
		desc.BlendEnable = true;
		desc.SrcBlend = D3D12_BLEND_ONE;
		desc.DestBlend = D3D12_BLEND_ONE;
		desc.BlendOp = D3D12_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;
	default:
		break;
	}
	return desc;
}
