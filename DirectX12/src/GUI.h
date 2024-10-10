#pragma once
#include <d3d12.h>


class GUI
{
private:
	static ID3D12DescriptorHeap* m_descriptorHeap;

public:
	static void Init(ID3D12Device* device);
	static void Update();
	static void Render(ID3D12GraphicsCommandList* cmdList);
	static void Destroy();
};