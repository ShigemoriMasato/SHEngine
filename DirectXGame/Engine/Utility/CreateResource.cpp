#include "CreateResource.h"

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
    //頂点リソース用のヒープの設定
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//uploadHeapを使う
    //頂点リソースの設定
    D3D12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);
    //バッファリソース、テクスチャの場合はまた別の設定をする
    bufferResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferResourceDesc.Width = sizeInBytes;
    //バッファの場合はこれにする決まり
    bufferResourceDesc.Height = 1;
    bufferResourceDesc.DepthOrArraySize = 1;
    bufferResourceDesc.MipLevels = 1;
    bufferResourceDesc.SampleDesc.Count = 1;
    //バッファの場合はこれにする決まり
    bufferResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ID3D12Resource* bufferResource = nullptr;

    HRESULT reason = device->GetDeviceRemovedReason();

    HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &bufferResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&bufferResource));
    assert(SUCCEEDED(hr));

    return bufferResource;
}
