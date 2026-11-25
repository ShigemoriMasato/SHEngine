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
		bool isDDS = path.extension() == ".dds" || path.extension() == ".CubeTexture";

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

void TextureData::CopyBuffer(const void*, size_t) {
	logger_->error("TextureData::CopyBuffer: Texture can't copy buffer. ({})", name_);
	assert(false && "TextureData::CopyBuffer: Texture can't copy buffer");
}

void TextureData::SetTextureResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState, bool attach) {
	resources_.clear();
	auto& res = resources_.emplace_back().res;
	if (attach) {
		res.Attach(resource);
	} else {
		res = resource;
	}

	// テクスチャは1枚なのでフレームごとの多重化はしない
	currentState_.assign(1, initialState);
	nextState_ = initialState;

	bufferType_ = uint8_t(BufferType::SRV);
	sizeInBytes_ = 0;
	strideInBytes_ = 0;
	num_ = 0;
}

void TextureData::CreateSRV(ID3D12Device* device, SRVManager* srvManager, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	auto& handle = srvHandles_.emplace_back(std::make_unique<SRVHandle>());
	// テクスチャはバインドレス(t8~)の範囲に置くため0から確保する
	handle->UpdateHandle(srvManager, 0);
	device->CreateShaderResourceView(GetResource(), &desc, handle->GetCPU());
	descriptorHandles_[BufferType::SRV].push_back(handle->GetGPU());
	bufferType_ |= uint8_t(BufferType::SRV);
}

void TextureData::CreateUAV(ID3D12Device* device, SRVManager* srvManager, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc) {
	auto& handle = uavHandles_.emplace_back(std::make_unique<SRVHandle>());
	handle->UpdateHandle(srvManager, 0);
	device->CreateUnorderedAccessView(GetResource(), nullptr, &desc, handle->GetCPU());
	descriptorHandles_[BufferType::UAV].push_back(handle->GetGPU());
	bufferType_ |= uint8_t(BufferType::UAV);
	unordered_ = true;
}

void TextureData::SetDebugName(const std::string& prefix) {
	name_ = prefix + " : " + std::to_string(debugTextureCount++);
	if (GetResource()) {
		GetResource()->SetName(ConvertString(name_).c_str());
	}
}

void TextureData::Create(uint32_t width, uint32_t height, Vector4 clearColor, Format format, bool unordered, ID3D12Device* device, SRVManager* srvManager) {
	DXGI_FORMAT dxgiformat = DXGI_FORMAT_UNKNOWN;
	switch (format) {
	case Format::R8_UNORM:
		dxgiformat = DXGI_FORMAT_R8_UNORM;
		break;
	case Format::R8G8B8A8_UNORM:
		dxgiformat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case Format::R32G32B32A32_UINT:
		dxgiformat = DXGI_FORMAT_R32G32B32A32_UINT;
		break;
	case Format::R32_UINT:
		dxgiformat = DXGI_FORMAT_R32_UINT;
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

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr) && "Failed to create off-screen resource");
	SetTextureResource(resource, D3D12_RESOURCE_STATE_COMMON, true);

	clearColor_ = clearColor;

	// metadataがないのでフォーマットとミップ数は手動設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = dxgiformat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//!mipmapを使うかどうかは今後要検討
	srvDesc.Texture2D.MipLevels = 1;
	CreateSRV(device, srvManager, srvDesc);

	width_ = width;
	height_ = height;

	SetDebugName("WindowTexture");

	if (unordered) {
		// UAVの作成
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = dxgiformat;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;
		CreateUAV(device, srvManager, uavDesc);
	}
}

void TextureData::Create(ID3D12Resource* resource, ID3D12Device* device, SRVManager* manager, uint32_t clearColor) {
	// SwapChainのバッファはPRESENT(=COMMON)状態で渡される
	SetTextureResource(resource, D3D12_RESOURCE_STATE_PRESENT, true);

	format_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// metadataがないのでフォーマットとミップ数は手動設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format_;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//!mipmapを使うかどうかは今後要検討
	srvDesc.Texture2D.MipLevels = 1;
	CreateSRV(device, manager, srvDesc);

	auto desc = resource->GetDesc();

	width_ = static_cast<uint32_t>(desc.Width);
	height_ = static_cast<uint32_t>(desc.Height);
	clearColor_ = ConvertColor(clearColor);

	SetDebugName("SwapChainTexture");
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

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heap,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr) && "Failed to create bitmap texture resource");
	SetTextureResource(resource, D3D12_RESOURCE_STATE_COPY_DEST, true);

	format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	width_ = width;
	height_ = height;

	//SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	CreateSRV(device, srvManager, srvDesc);

	SetDebugName("BitMapTexture");
}

DirectX::ScratchImage TextureData::Create(std::string filePath, ID3D12Device* device, SRVManager* srvManager) {
	std::filesystem::path path(filePath);
	DirectX::ScratchImage mipImages;
	bool createMipimage = false;

	std::string ddsFilePath = "Compiled/" + filePath.substr(0, filePath.find_last_of('.')) + ".dds";

	//読み込む対象がCompileされていない場合
	if (!std::filesystem::exists(ddsFilePath)) {
		if (!std::filesystem::exists("Assets/" + filePath)) {
			// 何処にもファイルが存在しないのでエラー
			return {};
		}

		//既にddsの場合はCompiledにコピーする
		if (path.extension() == ".dds" || path.extension() == ".CubeTexture") {
			std::filesystem::copy_file("Assets/" + filePath, "Compiled/" + filePath, std::filesystem::copy_options::overwrite_existing);
		} else {
			createMipimage = true;

			//ddsに変換してCompiledにコピーする
			DirectX::ScratchImage tmp;
			tmp = CreateMipImages("Assets/" + filePath);

			logger_->info("Compressing texture: {} width: {}, height: {}", filePath, tmp.GetMetadata().width, tmp.GetMetadata().height);

			HRESULT hr = DirectX::Compress(tmp.GetImages(), tmp.GetImageCount(), tmp.GetMetadata(),
				DXGI_FORMAT_BC7_UNORM_SRGB, DirectX::TEX_COMPRESS_DEFAULT | DirectX::TEX_COMPRESS_BC7_QUICK,
				DirectX::TEX_THRESHOLD_DEFAULT, mipImages);

			if (FAILED(hr)) {
				//とりあえずpntをそのまま使う
				logger_->warn("Failed to compress texture: " + filePath);
				mipImages = std::move(tmp);
			} else {

				std::filesystem::create_directories("Compiled/" + filePath.substr(0, filePath.find_last_of('/')));
				hr = DirectX::SaveToDDSFile(
					mipImages.GetImages(),
					mipImages.GetImageCount(),
					mipImages.GetMetadata(),
					DirectX::DDS_FLAGS_NONE,
					(ConvertString(ddsFilePath)).c_str());

				if (FAILED(hr)) {
					logger_->warn("Failed to save DDSTexture: " + filePath);
				}
			}
		}
	}

	// まだ作ってない場合はCompiledからddsを読み込む
	if (!createMipimage) {
		mipImages = CreateMipImages(ddsFilePath);
	}

	//TextureResourceを作成
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	SetTextureResource(CreateTextureResource(device, metadata), D3D12_RESOURCE_STATE_COPY_DEST, true);
	format_ = metadata.format;

	if (metadata.IsCubemap()) {
		type_ = Type::CubeTexture;
	} else if (metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE2D) {
		logger_->error("This Format is not supported. : " + filePath);
		return {};
	}

	//画像サイズの取得
	width_ = static_cast<uint32_t>(metadata.width);
	height_ = static_cast<uint32_t>(metadata.height);
	
	//metadataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (type_ == Type::CubeTexture) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//SRVを作成する
	CreateSRV(device, srvManager, srvDesc);

	SetDebugName("LoadTexture");
	// 表示名はファイル名にする
	filePath_ = filePath;
	name_ = path.filename().string();

	//テクスチャデータをアップロードするたに必要なデータを返す
	return mipImages;
}

void RenderTexture::CreateRTV(ID3D12Device* device, RTVManager* rtvManager) {
	rtvHandle_.UpdateHandle(rtvManager);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format_;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2Dテクスチャとしてよみこむ
	device->CreateRenderTargetView(GetResource(), &rtvDesc, rtvHandle_.GetCPU());
}

void DepthTexture::CreateDepth(uint32_t width, uint32_t height, ID3D12Device* device, SRVManager* srvManager, DSVManager* dsvManager) {
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr) && "Failed to create depth texture resource");
	SetTextureResource(resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	//DSVの作成
	dsvHandle_.UpdateHandle(dsvManager);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(GetResource(), &dsvDesc, dsvHandle_.GetCPU());

	//SRVの作成(R32_FLOATとして参照する)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	CreateSRV(device, srvManager, srvDesc);

	format_ = DXGI_FORMAT_R32_FLOAT;
	width_ = width;
	height_ = height;

	SetDebugName("DepthTexture");
}
