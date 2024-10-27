#pragma once
#include <d3d12.h>
#include <vector>

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

		void Increment();
		void Reset();
		inline D3D12_CPU_DESCRIPTOR_HANDLE Get() { return m_handle; }
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

		void Increment();
		void ResetToGraphicsRootDescriptorTableStart();
		inline D3D12_GPU_DESCRIPTOR_HANDLE Get() { return m_handle; }
	};


private:
	ID3D12DescriptorHeap* m_heap;
	CPUHandle m_cpuHandle;
	GPUHandle m_gpuHandle;

	static UINT s_handleIncrementSize;

public:
	Descriptor(UINT numDescriptors, const std::vector<int>& rootArgumentsOffset);
	~Descriptor();

	ID3D12DescriptorHeap*& GetHeap() { return m_heap; }

	CPUHandle GetCPUHandle() { return m_cpuHandle; }
	GPUHandle GetGPUHandle() { return m_gpuHandle; }

	static void Init(UINT size) { s_handleIncrementSize = size; }
};