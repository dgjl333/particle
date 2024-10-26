#pragma once
#include <vector>
#include <d3d12.h>

enum class RangeType
{
	CBV = 1 << 0,
	SRV = 1 << 1,
	UAV = 1 << 2
};

inline RangeType operator|(RangeType a, RangeType b)
{
	return (RangeType)((int)a | (int)b);
}


class RootSignatureManager
{
private:
	ID3D12RootSignature* m_signature;
	std::vector<D3D12_ROOT_PARAMETER> m_parameters;
	std::vector<D3D12_DESCRIPTOR_RANGE> m_ranges;
	D3D12_ROOT_SIGNATURE_DESC m_desc;

	int m_cbvRegister = 1;  //register 0 is reserved
	int m_srvRegister = 1;
	int m_uavRegister = 1;

	inline D3D12_ROOT_PARAMETER GetParameter(D3D12_DESCRIPTOR_RANGE* descriptorRanges, UINT numDescriptorRanges)
	{
		D3D12_ROOT_PARAMETER parameter = {};
		parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		parameter.DescriptorTable.pDescriptorRanges = descriptorRanges;
		parameter.DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
		return parameter;
	}

	inline D3D12_DESCRIPTOR_RANGE GetRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT baseShaderRegister)
	{
		D3D12_DESCRIPTOR_RANGE range = {};
		range.NumDescriptors = 1;
		range.RangeType = type;
		range.BaseShaderRegister = baseShaderRegister;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		return range;
	}

	inline bool HasFlag(RangeType value, RangeType flag)
	{
		return ((int)value & (int)flag) != 0;
	}

public:
	RootSignatureManager();
	~RootSignatureManager();

	void Serialize(const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers, UINT numStaticSamplers);

	void Add(RangeType type);

	ID3D12RootSignature* GetSignature() { return m_signature; }
};