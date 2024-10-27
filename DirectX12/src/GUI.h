#pragma once
#include <d3d12.h>


class GUI
{
private:
	static ID3D12DescriptorHeap* s_descriptorHeap;

public:
	static void Init();
	static void Update();
	static void Render(ID3D12GraphicsCommandList* cmdList);
	static void Destroy();
};