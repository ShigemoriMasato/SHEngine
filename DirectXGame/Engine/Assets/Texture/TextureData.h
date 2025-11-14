#pragma once
#include <string>
#include <d3d12.h>
#include <Core/View/SRVManager.h>
#include <utility>
#include <Utility/Vector.h>

class TextureData {
public:

	TextureData() = default;
	~TextureData() = default;

	//Window用のテクスチャを作成
	void Create(uint32_t width, uint32_t height, Vector4 clearColor, bool forSwapChain, ID3D12Device* device, SRVManager* srvManager);

	int GetOffset() const { return srvHandle_.GetOffset(); }
	ID3D12Resource* GetResource() const { return textureResource_.Get(); }
	std::pair<uint32_t, uint32_t> GetSize() const { return { width_, height_ }; }

private:

	friend class TextureManager;
	void Create(std::string filePath, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, SRVManager* srvManager);

private:

	uint32_t width_ = 0;
	uint32_t height_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;

	//CommandListが一度executeされたら削除するやつ
	Microsoft::WRL::ComPtr<ID3D12Resource> intermadiateResource_ = nullptr;

	SRVHandle srvHandle_{};

};

