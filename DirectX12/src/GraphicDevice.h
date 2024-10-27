#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>

class GraphicDevice
{
private:
	static D3D_FEATURE_LEVEL s_levels[5];

	static ID3D12Device* s_device;
	static IDXGIFactory6* s_dxgiFactory;

public:
	static void Init();
	static void Destroy();

	static ID3D12Device* GetDevice() { return  s_device; }
	static IDXGIFactory6* GetFactory() { return s_dxgiFactory; }
};