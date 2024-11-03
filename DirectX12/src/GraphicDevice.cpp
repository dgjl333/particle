#include "GraphicDevice.h"
#include <vector>
#include "Debug.h"

ID3D12Device* GraphicDevice::s_device = nullptr;
IDXGIFactory6* GraphicDevice::s_dxgiFactory = nullptr;

void GraphicDevice::Init()
{
#ifdef _DEBUG
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&s_dxgiFactory))))
	{
		print("Failed creating DXGI factory");
		exit(1);
	}
#else
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&s_dxgiFactory)))) exit(1);
#endif 

	std::vector<IDXGIAdapter*> allAdapters;
	IDXGIAdapter* adapter = nullptr;
	for (size_t i = 0; s_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		allAdapters.push_back(adapter);
	}

	for (const auto& adpt : allAdapters)
	{
		DXGI_ADAPTER_DESC desc = {};
		adpt->GetDesc(&desc);
		std::wstring description = desc.Description;
		if (description.find(L"NVIDIA") != std::string::npos)
		{
			adapter = adpt;
			break;
		}
	}

	for (D3D_FEATURE_LEVEL level : s_levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&s_device))))
		{
			break;
		}
	}
}

void GraphicDevice::Destroy()
{
	if(s_device) s_device->Release();
	if(s_dxgiFactory) s_dxgiFactory->Release();
}

D3D_FEATURE_LEVEL GraphicDevice::s_levels[5] = {
   D3D_FEATURE_LEVEL_12_2,
   D3D_FEATURE_LEVEL_12_1,
   D3D_FEATURE_LEVEL_12_0,
   D3D_FEATURE_LEVEL_11_1,
   D3D_FEATURE_LEVEL_11_0,
};


