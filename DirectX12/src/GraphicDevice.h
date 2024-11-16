#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class GraphicDevice
{
public:
	static void Init();

	static ID3D12Device* GetDevice()  { return  s_device.Get(); }
	static IDXGIFactory6* GetFactory()  { return s_dxgiFactory.Get(); }

private:
	static D3D_FEATURE_LEVEL s_levels[5];

	static ComPtr<ID3D12Device> s_device;
	static ComPtr<IDXGIFactory6> s_dxgiFactory;
};