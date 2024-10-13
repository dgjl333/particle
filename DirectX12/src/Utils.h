#pragma once
#include <string>
#include <Windows.h>

#define NUM_BACK_BUFFERS 2

namespace Utils
{
	 inline UINT64 AlignSize256(UINT64 size)
	{
		 return (size+ 0xff) & ~0xff;
	}


	inline D3D12_HEAP_PROPERTIES heapPropertiesUpload = {
		D3D12_HEAP_TYPE_UPLOAD,                 // Type
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,        // CPUPageProperty
		D3D12_MEMORY_POOL_UNKNOWN,              // MemoryPoolPreference
		0,                                      // CreationNodeMask
		0                                       // VisibleNodeMask
	};

	 inline D3D12_RESOURCE_DESC ResourceDesc(UINT64 width)
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = width;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		return desc;
	}

	inline std::wstring GetWStringFromString(const std::string& str)
	{
		int num = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);
		std::wstring wstr;
		wstr.resize(num);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num);
		return wstr;
	}

};