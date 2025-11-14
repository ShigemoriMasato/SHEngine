#pragma once
#include <Core/View/DSVManager.h>
#include <Core/View/RTVManager.h>
#include <Assets/Texture/TextureData.h>
#include <Utility/Vector.h>

class Display {
public:

	static void StaticInitialize(ID3D12Device* device, RTVManager* rtvManager, DSVManager* dsvManager);

	Display() = default;
	~Display() = default;

	void Initialize(TextureData* data, uint32_t clearColor);
	void DrawReady(ID3D12GraphicsCommandList* commandList, bool isClear);

	void EditBarrier(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

	ID3D12Resource* GetTextureResource() const { return textureResource_; }
	RTVHandle GetRTVHandle() const { return rtvHandle_; }

private:

	static ID3D12Device* device_;
	static RTVManager* rtvManager_;
	static DSVManager* dsvManager_;

private:

	RTVHandle rtvHandle_;
	DSVHandle dsvHandle_;

	D3D12_RESOURCE_STATES resourceState_;
	
	ID3D12Resource* textureResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;

	Vector4 clearColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	int width_;
	int height_;
};
