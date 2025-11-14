#include "Function.h"

ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible) {
    //ディスクリプタヒープの設定
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = type;	//ディスクリプタの種類
    descriptorHeapDesc.NumDescriptors = numDescriptors;	//ディスクリプタの数
    descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//シェーダーからアクセスできるようにする
    ID3D12DescriptorHeap* descriptorHeap = nullptr;
    HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
    assert(SUCCEEDED(hr));
    return descriptorHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDesscriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (descriptorSize * index);
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDesscriptorHandle(ID3D12DescriptorHeap* heap, uint32_t descriptorSize, UINT index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += (descriptorSize * index);
    return handle;
}
