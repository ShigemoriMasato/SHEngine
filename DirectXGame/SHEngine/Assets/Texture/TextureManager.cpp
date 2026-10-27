#include "TextureManager.h"
#include <Utility/Color.h>
#include <Utility/DirectUtilFuncs.h>
#include <DirectXTex/d3dx12.h>
#include <Utility/SearchFile.h>

using namespace SHEngine;

namespace {

	[[nodiscard]]
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		HRESULT hr = DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
		assert(SUCCEEDED(hr));
		uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
		ID3D12Resource* intermediateResource = Func::CreateBufferResource(device, intermediateSize);
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

SHEngine::TextureManager::~TextureManager() {
}

void TextureManager::Initialize(DXDevice* device) {
	device_ = device;
	srvManager_ = device->GetSRVManager();

	logger_ = GetLogger("Engine");

	LoadTexture(".EngineResource/Texture/white1x1.png");
	LoadTexture(".EngineResource/Texture/uvChecker.png");
	errorTextureHandle_ = LoadTexture(".EngineResource/Texture/error.png");

	LoadTexture(".EngineResource/Texture/File.png");
	LoadTexture(".EngineResource/Texture/Folder.png");
	LoadTexture(".EngineResource/Texture/Model.png");
	LoadTexture(".EngineResource/Texture/Texture.png");
}

void TextureManager::AllTextureClear() {
	textureDataList_.clear();
}

void TextureManager::LoadAllTextures() {
	auto files = SearchFilePathsAddChild("Assets/Texture/", ".png");

	for (const auto& filePath : files) {
		LoadTexture("Texture/" + filePath);
	}
}

TextureData* TextureManager::GetTextureData(std::string filePath) const {
	auto it = loadedTexturePaths_.find(filePath);
	if (it != loadedTexturePaths_.end()) {
		return GetTextureData(it->second);
	}
	return GetTextureData(errorTextureHandle_);
}

int TextureManager::LoadTexture(const std::string& filePath) {
	auto textureData = std::make_unique<TextureData>();

	//ファイルが存在しなかったらエラーテクスチャを返す

	//すでに読み込んでいたらそのハンドルを返す
	auto it = loadedTexturePaths_.find(filePath);
	if (it != loadedTexturePaths_.end()) {
		return it->second;
	}

	auto scratchImage = textureData->Create(filePath, device_->GetDevice(), srvManager_);

	if (scratchImage.GetImageCount() == 0) {
		return errorTextureHandle_;
	}

	int offset = textureData->GetHandle();

	uploadStandby_.emplace_back(offset, std::move(scratchImage));

	textureData->textureManager_ = this;
	textureDataList_[offset] = std::move(textureData);
	CheckMaxCount(offset);
	loadedTexturePaths_[filePath] = offset;

	return offset;
}

int TextureManager::CreateWindowTexture(uint32_t width, uint32_t height, uint32_t clearColor, Format format, bool unordered) {
	auto textureData = std::make_unique<TextureData>();
	Vector4 clearColorVec = ConvertColor(clearColor);
	textureData->Create(width, height, clearColorVec, format, unordered, device_->GetDevice(), srvManager_);
	int offset = textureData->GetHandle();
	textureData->textureManager_ = this;
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int TextureManager::CreateSwapChainTexture(ID3D12Resource* resource, uint32_t clearColor) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(resource, device_->GetDevice(), srvManager_, clearColor);
	textureData->textureManager_ = this;
	int offset = textureData->GetHandle();
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int SHEngine::TextureManager::CreateDepthTexture(ID3D12Resource* resource) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(resource, device_->GetDevice(), srvManager_);
	textureData->textureManager_ = this;
	int offset = textureData->GetHandle();
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int TextureManager::CreateBitmapTexture(uint32_t width, uint32_t height, DirectX::ScratchImage& scratchImage) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(width, height, device_->GetDevice(), srvManager_);
	int offset = textureData->GetHandle();

	uploadStandby_.emplace_back(offset, std::move(scratchImage));

	textureData->textureManager_ = this;
	textureDataList_[offset] = std::move(textureData);

	return offset;
}

void TextureManager::DeleteTexture(int handle) {
	auto it = textureDataList_.find(handle);
	if (it != textureDataList_.end()) {
		textureDataList_.erase(it);
	}
}

void TextureManager::DeleteTexture(TextureData* textureData) {
	for (const auto& [handle, data] : textureDataList_) {
		if (data.get() == textureData) {
			textureDataList_.erase(handle);
			return;
		}
	}
}

TextureData* TextureManager::GetTextureData(int handle) const {
	return textureDataList_.at(handle).get();
}

void TextureManager::UploadResources(ID3D12GraphicsCommandList* cmdList) {
	//中間リソースがなければ何もしない
	if (uploadStandby_.empty()) {
		return;
	}

	intermediateResources_.reserve(intermediateResources_.size() + uploadStandby_.size());

	for (auto& [offset, scratchImage] : uploadStandby_) {
		auto textureData = textureDataList_[offset].get();
		auto intermediateResource = UploadTextureData(textureData->GetResource(), scratchImage, device_->GetDevice(), cmdList);
		Resource res;
		res.res.Attach(intermediateResource);
		intermediateResources_.push_back(res);
	}

	uploadStandby_.clear();
}

void SHEngine::TextureManager::ClearIntermediateResource() {
	intermediateResources_.clear();
}

void TextureManager::CheckMaxCount(int offset) {
	if (offset >= maxTextureCount) {
		assert(false && "I can't read Texture more!");
	}
}
