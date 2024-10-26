#include "RootSignatureManager.h"
#include "Debug.h"
#include "GraphicDevice.h"

RootSignatureManager::RootSignatureManager():
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

void RootSignatureManager::Add(RangeType type)
{
	int orginalSize = m_ranges.size();
	if (HasFlag(type, RangeType::CBV))
	{
		m_ranges.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_cbvRegister));
		m_cbvRegister++;
	}

	if (HasFlag(type, RangeType::SRV))
	{
		m_ranges.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_srvRegister));
		m_srvRegister++;
	}

	if (HasFlag(type, RangeType::UAV))
	{
		m_ranges.push_back(GetRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, m_uavRegister));
		m_uavRegister++;
	}

	int newSize = m_ranges.size();
	int addedNum = newSize - orginalSize;
	m_parameters.push_back(GetParameter(&m_ranges[orginalSize], addedNum));
}

void RootSignatureManager::Serialize(const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers, UINT numStaticSamplers)
{
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
}

RootSignatureManager::~RootSignatureManager()
{
	m_signature->Release();
}

