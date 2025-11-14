#include "TextureData.h"
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/d3dx12.h>
#include <Utility/ConvertString.h>
#include <Utility/CreateResource.h>

namespace {
    DirectX::ScratchImage CreateMipImages(const std::string& filePath) {
        //テクスチャファイルを読んでプログラムで扱えるようにする
        DirectX::ScratchImage image{};
        std::wstring filePathW = ConvertString(filePath);
        HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
        assert(SUCCEEDED(hr) && "Failed to Open TextureFile");

        DirectX::ScratchImage mipImages{};
        //ミニマップの作成(画像サイズが最小の場合、作成手順を飛ばす)
        if (image.GetMetadata().mipLevels > 1) {
            hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
            assert(SUCCEEDED(hr));
        } else {
            mipImages = std::move(image);
        }
        return mipImages;
    }

    ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
        //metadataを基にResourceの設定
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = UINT(metadata.width);
        resourceDesc.Height = UINT(metadata.height);
        resourceDesc.MipLevels = UINT16(metadata.mipLevels);
        resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
        resourceDesc.Format = metadata.format;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

        //利用するHeapの設定。非常に特殊な運用。
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        //Resourceの生成
        ID3D12Resource* resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE,
            &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
            IID_PPV_ARGS(&resource));
        assert(SUCCEEDED(hr));
        return resource;
    }

    [[nodiscard]]
    ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
        std::vector<D3D12_SUBRESOURCE_DATA> subresources;
        DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
        uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
        ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
        UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = texture;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);
        return intermediateResource;
    }
}

void TextureData::Create(uint32_t width, uint32_t height, Vector4 clearColor, bool forSwapChain, ID3D12Device* device, SRVManager* srvManager) {
    //PostEffect用のリソースの作成
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;              // defaultのヒープを使用

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = forSwapChain ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    for (int i = 0; i < 4; ++i) {
		clearValue.Color[i] = clearColor[i];
    }

    HRESULT hr = device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(&textureResource_)
    );
    assert(SUCCEEDED(hr) && "Failed to create off-screen resource");

    // metadataがないのでフォーマットとミップ数は手動設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //!mipmapを使うかどうかは今後要検討
    srvDesc.Texture2D.MipLevels = 1;

    // SRV用ディスクリプタ位置を確保
    srvHandle_.UpdateHandle(srvManager);

    // SRVを作成
    device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

    width_ = width;
    height_ = height;
}

void TextureData::Create(std::string filePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SRVManager* srvManager) {
    //TextureResourceを作成
    DirectX::ScratchImage mipImages = CreateMipImages(filePath);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    textureResource_.Attach(CreateTextureResource(device, metadata));
    intermadiateResource_.Attach(UploadTextureData(textureResource_.Get(), mipImages, device, commandList));

    //画像サイズの取得
    width_ = static_cast<int>(metadata.width);
    height_ = static_cast<int>(metadata.height);

    //metadataをもとにSRVの設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    //SRVを作成するDescriptorHeapの場所を決める
    srvHandle_.UpdateHandle(srvManager, true);

    //SRVを作成する
    device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());
}
