#include "TextureData.h"
#include <DirectXTex/d3dx12.h>
#include <Utility/ConvertString.h>
#include <Assets/Texture/TextureManager.h>
#include <Utility/DirectUtilFuncs.h>
#include <Utility/Color.h>
#include <filesystem>

using namespace Microsoft::WRL;
using namespace SHEngine;

namespace {
	DirectX::ScratchImage CreateMipImages(const std::string& filePath) {
		//DDSファイルかどうかの判定
		std::filesystem::path path(filePath);
		bool isDDS = path.extension() == ".dds" || path.extension() == ".DDS";

		DirectX::ScratchImage image{};
		HRESULT hr = S_OK;
		if (isDDS) {
			hr = DirectX::LoadFromDDSFile(ConvertString(filePath).c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		} else {
			hr = DirectX::LoadFromWICFile(ConvertString(filePath).c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		}

		//成功したかどうかの判断
		assert(SUCCEEDED(hr) && "Failed to Open TextureFile");

		DirectX::ScratchImage mipImages{};
		//ミニマップの作成(画像サイズが最小の場合または既に圧縮済みの場合、作成手順を飛ばす)
		if (image.GetMetadata().mipLevels > 1 && !DirectX::IsCompressed(image.GetMetadata().format)) {
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

}

void TextureData::Release() {
	textureManager_->DeleteTexture(this);
}

void TextureData::Create(uint32_t width, uint32_t height, Vector4 clearColor, Format format, bool unordered, ID3D12Device* device, SRVManager* srvManager) {
	DXGI_FORMAT dxgiformat;
	switch (format) {
	case Format::R8:
		dxgiformat = DXGI_FORMAT_R8_UNORM;
		break;
	case Format::R16G16B16A16:
		dxgiformat = DXGI_FORMAT_R16G16B16A16_UNORM;
		break;
	}

	format_ = dxgiformat;
	unordered_ = unordered;

	//OffScreen用のリソースの作成
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = dxgiformat;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if (unordered) {
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;              // defaultのヒープを使用

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = dxgiformat;
	for (int i = 0; i < 4; ++i) {
		clearValue.Color[i] = clearColor[i];
	}

	HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, 
		D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&textureResource_));
	assert(SUCCEEDED(hr) && "Failed to create off-screen resource");

	clearColor_ = clearColor;

	// metadataがないのでフォーマットとミップ数は手動設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxgiformat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//!mipmapを使うかどうかは今後要検討
	srvDesc.Texture2D.MipLevels = 1;


	// SRV用ディスクリプタ位置を確保
	srvHandle_.UpdateHandle(srvManager, 0);

	// SRVを作成
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

	width_ = width;
	height_ = height;

	textureResource_->SetName(LPCWSTR(ConvertString("WindowTexture : " + std::to_string(debugTextureCount++)).c_str()));

	if (unordered) {
		// UAVの作成
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = dxgiformat;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;
		// UAV用ディスクリプタ位置を確保
		uavHandle_.UpdateHandle(srvManager, 0);
		// UAVを作成
		device->CreateUnorderedAccessView(textureResource_.Get(), nullptr, &uavDesc, uavHandle_.GetCPU());
	}
}

void TextureData::Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager, uint32_t clearColor) {
	textureResource_.Attach(resource);

	format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// metadataがないのでフォーマットとミップ数は手動設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format_;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//!mipmapを使うかどうかは今後要検討
	srvDesc.Texture2D.MipLevels = 1;

	// SRV用ディスクリプタ位置を確保
	srvHandle_.UpdateHandle(manager, 0);

	// SRVを作成
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

	auto desc = resource->GetDesc();

	width_ = static_cast<uint32_t>(desc.Width);
	height_ = static_cast<uint32_t>(desc.Height);
	clearColor_ = ConvertColor(clearColor);

	textureResource_->SetName(LPCWSTR(ConvertString("SwapChainTexture : " + std::to_string(debugTextureCount++)).c_str()));
}

void TextureData::Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager) {
	textureResource_.Attach(resource);

	// metadataがないのでフォーマットとミップ数は手動設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//!mipmapを使うかどうかは今後要検討
	srvDesc.Texture2D.MipLevels = 1;

	// SRV用ディスクリプタ位置を確保
	srvHandle_.UpdateHandle(manager, 0);

	// SRVを作成
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

	auto desc = resource->GetDesc();

	width_ = static_cast<uint32_t>(desc.Width);
	height_ = static_cast<uint32_t>(desc.Height);

	textureResource_->SetName(LPCWSTR(ConvertString("DepthTexture : " + std::to_string(debugTextureCount++)).c_str()));
}

void TextureData::Create(uint32_t width, uint32_t height, ID3D12Device* device, SRVManager* srvManager) {
	//TextureResource
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);

	device->CreateCommittedResource(
		&heap,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&textureResource_)
	);

	//SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvHandle_.UpdateHandle(srvManager, 0);
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

	textureResource_->SetName(LPCWSTR(ConvertString("BitMapTexture : " + std::to_string(debugTextureCount++)).c_str()));

}

DirectX::ScratchImage TextureData::Create(std::string filePath, ID3D12Device* device, SRVManager* srvManager) {
	std::filesystem::path path(filePath);
	if (path.extension() == ".dds") {
		type_ = Type::DDS;
	}

	//TextureResourceを作成
	DirectX::ScratchImage mipImages = CreateMipImages(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	textureResource_.Attach(CreateTextureResource(device, metadata));

	//画像サイズの取得
	width_ = static_cast<uint32_t>(metadata.width);
	height_ = static_cast<uint32_t>(metadata.height);
	
	//metadataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (type_ == Type::DDS) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	srvHandle_.UpdateHandle(srvManager, 0);

	//SRVを作成する
	device->CreateShaderResourceView(textureResource_.Get(), &srvDesc, srvHandle_.GetCPU());

	textureResource_->SetName(LPCWSTR(ConvertString("LoadTexture : " + std::to_string(debugTextureCount++)).c_str()));

	//テクスチャデータをアップロードするたに必要なデータを返す
	return mipImages;
}
