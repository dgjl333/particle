#include "RootSignature.h"
#include "Debug.h"
#include "GraphicDevice.h"
#include <format>

RootSignature::RootSignature():
	m_desc({}), m_signature(nullptr)
{
	D3D12_ROOT_DESCRIPTOR sharedDesc = {};     //internal shared cb resource
	sharedDesc.RegisterSpace = 0;
	sharedDesc.ShaderRegister = 0;

	D3D12_ROOT_PARAMETER sharedParameter = {};
	sharedParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	sharedParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	sharedParameter.Descriptor = sharedDesc;

	m_parameters.push_back(sharedParameter);
	m_ranges.reserve(100);
}

void RootSignature::Add(RangeType type)
{
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	if (HasFlag(type, RangeType::CBV))
	{
		range.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_cbvRegister));
		m_cbvRegister++;
	}

	if (HasFlag(type, RangeType::SRV))
	{
		range.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_srvRegister));
		m_srvRegister++;
	}

	if (HasFlag(type, RangeType::UAV))
	{
		range.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, m_uavRegister));
		m_uavRegister++;
	}
	m_ranges.push_back(std::move(range));
}

void RootSignature::Serialize(const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers, UINT numStaticSamplers)
{

	m_numDescriptors = m_parameters.size();
	m_rootArgumentsOffsets.push_back(m_numDescriptors);

	m_parameters.reserve(m_parameters.size() + m_ranges.size());
	for (std::vector<D3D12_DESCRIPTOR_RANGE>& range : m_ranges)
	{
		m_parameters.push_back(GetParameter(range.data(), range.size()));
		m_numDescriptors += range.size();
		m_rootArgumentsOffsets.push_back(range.size());
	}

	m_desc.pStaticSamplers = pStaticSamplers;
	m_desc.NumStaticSamplers = numStaticSamplers;
	m_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	m_desc.pParameters = m_parameters.data();
	m_desc.NumParameters = m_parameters.size();

	ID3D10Blob* rootSigatureBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	D3D12SerializeRootSignature(&m_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigatureBlob, &errorBlob);
	if (errorBlob)
	{
		char* errorMessage = (char*)(errorBlob->GetBufferPointer());
		print("Error serializing root signature:", errorMessage);
		errorBlob->Release();
		exit(1);
	}
	GraphicDevice::GetDevice()->CreateRootSignature(0, rootSigatureBlob->GetBufferPointer(), rootSigatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_signature));
	rootSigatureBlob->Release();

	DebugPrintLayout();

	m_parameters.~vector();
	m_ranges.~vector();
}

void RootSignature::DebugPrintLayout()
{
#ifdef _DEBUG
	for (int i = 0; i < m_parameters.size(); i++)
	{
		std::string message = "";
		if (m_parameters[i].ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			std::string typeName = DebugGetInfoType(m_parameters[i].ParameterType);
			message += typeName + DebugGetShaderRegister(typeName, m_parameters[i].Descriptor.ShaderRegister);
		}
		else
		{
			if (m_parameters[i - 1].ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				print("------------Descriptor Table---------------");
			}
			for (int j = 0; j < m_parameters[i].DescriptorTable.NumDescriptorRanges; j++)
			{
				auto range = m_parameters[i].DescriptorTable.pDescriptorRanges[j];
				std::string typeName = DebugGetInfoType(range.RangeType);
				message += typeName + DebugGetShaderRegister(typeName, range.BaseShaderRegister) + " ";
			}
		}
		print(message);
	}
#endif
}

std::string RootSignature::DebugGetInfoType(D3D12_DESCRIPTOR_RANGE_TYPE type)
{
	switch (type)
	{
	case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
		return "SRV";
	case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
		return "UAV";
	case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
		return "CBV";
	default:
		return "Unknown Type";
	}
}

std::string RootSignature::DebugGetInfoType(D3D12_ROOT_PARAMETER_TYPE type)
{
	switch (type)
	{
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		return "CBV";
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
		return "SRV";
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		return "UAV";
	default:
		return "Unknown Type";
	}
}

std::string RootSignature::DebugGetShaderRegister(const std::string& typeName, UINT shaderRegister)
{
	auto firstCharacter = typeName[0];
	if (firstCharacter == 'C') return std::format("(b{})", shaderRegister);
	else if (firstCharacter == 'S') return std::format("(t{})", shaderRegister);
	else if (firstCharacter == 'U') return std::format("(u{})", shaderRegister);
	else return "Unknown Register";
}