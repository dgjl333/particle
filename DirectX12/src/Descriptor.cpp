#include "Descriptor.h"
#include "GraphicDevice.h"

UINT Descriptor::s_handleIncrementSize = 0;

Descriptor::Descriptor(UINT numDescriptors, const std::vector<int>& rootArgumentsOffset):
	m_heap(nullptr), m_cpuHandle({}), m_gpuHandle()
{
	D3D12_DESCRIPTOR_HEAP_DESC  HeapDesc = {};
	HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HeapDesc.NodeMask = 0;
	HeapDesc.NumDescriptors = numDescriptors;  //num of cbv_srv_uav total amount
	HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	GraphicDevice::GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&m_heap));

	m_cpuHandle = CPUHandle(m_heap->GetCPUDescriptorHandleForHeapStart());
	m_gpuHandle = GPUHandle(m_heap->GetGPUDescriptorHandleForHeapStart(), rootArgumentsOffset);
}

Descriptor::~Descriptor()
{
	m_heap->Release();
}

Descriptor::CPUHandle::CPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle):
	m_initHandle(handle), m_handle(handle) {}


D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::CPUHandle::Increment()
{
	m_handle.ptr += s_handleIncrementSize;
	return m_handle;
}

void Descriptor::CPUHandle::Reset()
{
	m_handle = m_initHandle;
}

Descriptor::GPUHandle::GPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle, const std::vector<int>& rootArgumentsOffset) :
	m_initHandle(handle), m_handle(handle), m_index(0)
{
	m_initHandle.ptr += rootArgumentsOffset[0] * s_handleIncrementSize;
	m_handle = m_initHandle;

	for (int i = 1; i < rootArgumentsOffset.size() - 1; i++) 
	{
		m_rootArgumentsOffsets.push_back(rootArgumentsOffset[i]);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::GPUHandle::Increment()
{
	m_handle.ptr += m_rootArgumentsOffsets[m_index] * s_handleIncrementSize;
	m_index++;
	return m_handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::GPUHandle::ResetToGraphicsRootDescriptorTableStart()
{
	m_handle = m_initHandle;
	m_index = 0;
	return m_handle;
}