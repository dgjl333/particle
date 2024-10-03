#pragma once
#include <iostream>
#include <Windows.h>
#include <d3d12.h>

template<typename T> void print(const T& text)
{
#ifdef _DEBUG
    std::cout << text << std::endl;
#endif
}

template<typename T, typename... Args>
void print(const T& text, const Args&... args)
{
#ifdef _DEBUG
	std::cout << text << " ";
	print(args...);
#endif
}

inline void EnableDebug()
{
#ifdef _DEBUG
	ID3D12Debug* debug = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	debug->EnableDebugLayer();
	debug->Release();
#endif
}
