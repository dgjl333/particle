#pragma once
#include <d3d12.h>
#include <vector>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;

class Descriptor
{
public:
	class CPUHandle
	{
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_initHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE m_handle;

	public:
		CPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);

		void Reset();
		D3D12_CPU_DESCRIPTOR_HANDLE Increment();
		inline D3D12_CPU_DESCRIPTOR_HANDLE Get() const { return m_handle; }
	};

	class GPUHandle
	{
	private:
		std::vector<int> m_rootArgumentsOffsets;
		int m_index;

		D3D12_GPU_DESCRIPTOR_HANDLE m_initHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_handle;

	public:
		GPUHandle() = default;
		GPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle, const std::vector<int>& rootArgumentsOffset);

		D3D12_GPU_DESCRIPTOR_HANDLE ResetToGraphicsRootDescriptorTableStart();
		D3D12_GPU_DESCRIPTOR_HANDLE Increment();
		inline D3D12_GPU_DESCRIPTOR_HANDLE Get() const { return m_handle; }
	};

	Descriptor(UINT numDescriptors, const std::vector<int>& rootArgumentsOffset);

	ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }
	ID3D12DescriptorHeap** GetHeapAddress() { return m_heap.GetAddressOf(); }

	CPUHandle GetCPUHandle() const { return m_cpuHandle; }
	GPUHandle GetGPUHandle() const { return m_gpuHandle; }

	static void Init(UINT size) { s_handleIncrementSize = size; }

private:
	ComPtr<ID3D12DescriptorHeap> m_heap;
	CPUHandle m_cpuHandle;
	GPUHandle m_gpuHandle;

	static UINT s_handleIncrementSize;
};