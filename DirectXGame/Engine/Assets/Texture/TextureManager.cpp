#include "TextureManager.h"
#include <Utility/Color.h>

void TextureManager::Initialize(DXDevice* device, SRVManager* srvManager) {
	device_ = device;
	srvManager_ = srvManager;
}

int TextureManager::LoadTexture(const std::string& filePath) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(filePath, device_->GetDevice(), commandList_, srvManager_);
	int offset = textureData->GetOffset();
	textureDatas_[offset] = std::move(textureData);
	return offset;
}

int TextureManager::CreateWindowTexture(uint32_t width, uint32_t height, uint32_t clearColor, bool forSwapChain) {
	auto textureData = std::make_unique<TextureData>();
	Vector4 clearColorVec = ConvertColor(clearColor);
	textureData->Create(width, height, clearColorVec, forSwapChain, device_->GetDevice(), srvManager_);
	int offset = textureData->GetOffset();
	textureDatas_[offset] = std::move(textureData);
	return offset;
}

TextureData* TextureManager::GetTextureData(int handle) {
	return textureDatas_[handle].get();
}
