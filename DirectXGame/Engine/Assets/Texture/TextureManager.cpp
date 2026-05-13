#include "TextureManager.h"
#include <Utility/Color.h>
#include <Utility/DirectUtilFuncs.h>
#include <DirectXTex/d3dx12.h>
#include <Utility/SearchFile.h>

using namespace SHEngine;

SHEngine::TextureManager::~TextureManager() {
	cmdObject_->WaitForGPUIdle();
}

void TextureManager::Initialize(DXDevice* device, Command::Manager* manager) {
	device_ = device;
	cmdObject_ = manager->CreateCommandObject(Command::Type::Texture, 1);
	srvManager_ = device->GetSRVManager();
	manager_ = manager;

	cmdObject_->WaitForGPUIdle();
	cmdObject_->ResetCommandList();

	logger_ = getLogger("Engine");

	LoadTexture("Assets/.EngineResource/Texture/white1x1.png");
	LoadTexture("Assets/.EngineResource/Texture/uvChecker.png");
	errorTextureHandle_ = LoadTexture("Assets/.EngineResource/Texture/error.png");
}

void TextureManager::AllTextureClear() {
	textureDataList_.clear();
}

void TextureManager::LoadAllTextures() {
	auto files = SearchFilePathsAddChild("Assets/Texture/", ".png");
	
	for (const auto& filePath : files) {
		LoadTexture(filePath);
	}
}

TextureData* TextureManager::GetTextureData(std::string filePath) {
	std::string formatFirst = "Assets/";
	std::string factFilePath = "";
	if (filePath.length() < formatFirst.length()) {
		factFilePath = "Assets/Texture/" + filePath;
	} else {
		for (int i = 0; i < formatFirst.length(); ++i) {
			if (filePath[i] != formatFirst[i]) {
				factFilePath = "Assets/Texture/" + filePath;
				break;
			}

			if (i == formatFirst.length() - 1) {
				factFilePath = filePath;
			}
		}
	}

	auto it = loadedTexturePaths_.find(factFilePath);
	if (it != loadedTexturePaths_.end()) {
		GetTextureData(it->second);
	}
	return GetTextureData(errorTextureHandle_);
}

int TextureManager::LoadTexture(const std::string& filePath) {
	auto textureData = std::make_unique<TextureData>();

	std::string formatFirst = "Assets/";
	std::string factFilePath = "";
	if (filePath.length() < formatFirst.length()) {
		factFilePath = "Assets/Texture/" + filePath;
	} else {
		for (int i = 0; i < formatFirst.length(); ++i) {
			if (filePath[i] != formatFirst[i]) {
				factFilePath = "Assets/Texture/" + filePath;
				break;
			}

			if (i == formatFirst.length() - 1) {
				factFilePath = filePath;
			}
		}
	}

	//ファイルが存在しなかったらエラーテクスチャを返す
	if (!std::filesystem::exists(factFilePath)) {
		logger_->error("Texture File is Not Found: {}", factFilePath);
		return errorTextureHandle_;
	}

	//すでに読み込んでいたらそのハンドルを返す
	auto it = loadedTexturePaths_.find(factFilePath);
	if (it != loadedTexturePaths_.end()) {
		return it->second;
	}

	auto cmdList = cmdObject_->GetCommandList();
	intermediateResources_.push_back(textureData->Create(factFilePath, device_->GetDevice(), srvManager_, cmdList));
	int offset = textureData->GetOffset();
	textureData->textureManager_ = this;
	textureDataList_[offset] = std::move(textureData);
	CheckMaxCount(offset);
	loadedTexturePaths_[factFilePath] = offset;
	return offset;
}

int TextureManager::CreateWindowTexture(uint32_t width, uint32_t height, uint32_t clearColor) {
	auto textureData = std::make_unique<TextureData>();
	Vector4 clearColorVec = ConvertColor(clearColor);
	textureData->Create(width, height, clearColorVec, device_->GetDevice(), srvManager_);
	int offset = textureData->GetOffset();
	textureData->textureManager_ = this;
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int TextureManager::CreateSwapChainTexture(ID3D12Resource* resource, uint32_t clearColor) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(resource, device_->GetDevice(), srvManager_, clearColor);
	textureData->textureManager_ = this;
	int offset = textureData->GetOffset();
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int SHEngine::TextureManager::CreateDepthTexture(ID3D12Resource* resource) {
	auto textureData = std::make_unique<TextureData>();
	textureData->Create(resource, device_->GetDevice(), srvManager_);
	textureData->textureManager_ = this;
	int offset = textureData->GetOffset();
	textureDataList_[offset] = std::move(textureData);
	return offset;
}

int TextureManager::CreateBitmapTexture(uint32_t width, uint32_t height, std::vector<uint32_t> colorMap) {
	auto textureData = std::make_unique<TextureData>();
	auto cmdList = cmdObject_->GetCommandList();
	auto data = textureData->Create(width, height, colorMap, device_->GetDevice(), srvManager_, cmdList);
	intermediateResources_.push_back(data);
	textureData->textureManager_ = this;
	int offset = textureData->GetOffset();
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

TextureData* TextureManager::GetTextureData(int handle) {
	return textureDataList_[handle].get();
}

void TextureManager::UploadResources() {
	//中間リソースがなければ何もしない
	if (intermediateResources_.empty()) {
		return;
	}

	//実行
	manager_->Execute(Command::Type::Texture, { cmdObject_.get() });

	//コマンドリストをリセット
	cmdObject_->ResetCommandList();

	//中間リソースをクリア
	intermediateResources_.clear();
}

void TextureManager::CheckMaxCount(int offset) {
	if (offset >= maxTextureCount) {
		assert(false && "I can't read Texture more!");
	}
}
