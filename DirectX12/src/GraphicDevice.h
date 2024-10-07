#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>

class GraphicDevice
{
private:
	static D3D_FEATURE_LEVEL levels[5];

	ID3D12Device* m_device = nullptr;
	IDXGIFactory6* m_dxgiFactory = nullptr;

public:
	GraphicDevice();
	~GraphicDevice();

	ID3D12Device* GetDevice() { return m_device; }
	IDXGIFactory6* GetFactory() { return m_dxgiFactory; }
};