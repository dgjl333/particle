#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>

class GraphicDevice
{
private:
	static D3D_FEATURE_LEVEL levels[5];

	static ID3D12Device* m_device;
	static IDXGIFactory6* m_dxgiFactory;

public:
	static void Init();
	static void Destroy();

	static ID3D12Device* GetDevice() { return  m_device; }
	static IDXGIFactory6* GetFactory() { return m_dxgiFactory; }
};