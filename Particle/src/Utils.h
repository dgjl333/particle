#pragma once
#include <string>
#include <Windows.h>

#define NUM_BACK_BUFFERS 2

namespace Utils
{
	 inline UINT64 AlignSize256(UINT64 size)
	{
		 return (size+ 255) & ~255;
	}

	inline D3D12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE type)
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = type;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 0;
		prop.VisibleNodeMask = 0;
		return prop;
	}

	 inline D3D12_RESOURCE_DESC ResourceDesc(UINT64 width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = width;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = flags;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		return desc;
	}

	 inline D3D12_RESOURCE_BARRIER ResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
	 {
		 D3D12_RESOURCE_BARRIER barrier = {};
		 barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		 barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		 barrier.Transition.pResource = pResource;
		 barrier.Transition.StateBefore = before;
		 barrier.Transition.StateAfter = after;
		 barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		 return barrier;
	 }

	inline std::wstring WStringFromString(const std::string& str)
	{
		int num = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
		std::wstring wstr;
		wstr.resize(num);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num);
		return wstr;
	}

};