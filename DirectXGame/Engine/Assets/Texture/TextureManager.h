#pragma once
#include "TextureData.h"
#include <map>
#include <memory>

class TextureManager {
public:

	TextureManager() = default;
	~TextureManager() = default;

	void Initialize(DXDevice* device, SRVManager* srvManager);
	void SetCommandList(ID3D12GraphicsCommandList* commandList) { commandList_ = commandList; }

	int LoadTexture(const std::string& filePath);
	int CreateWindowTexture(uint32_t width, uint32_t height, uint32_t clearColor, bool forSwapChain = false);

	TextureData* GetTextureData(int handle);
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTextureResource(int handle) {
		return textureDatas_[handle]->textureResource_;
	}

private:

	DXDevice* device_ = nullptr;
	SRVManager* srvManager_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;

	std::map<int, std::unique_ptr<TextureData>> textureDatas_;

};

