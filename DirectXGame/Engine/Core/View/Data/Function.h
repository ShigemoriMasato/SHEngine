#pragma once
#include <Core/Device/DXDevice.h>

ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDesscriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index);

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDesscriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index);
