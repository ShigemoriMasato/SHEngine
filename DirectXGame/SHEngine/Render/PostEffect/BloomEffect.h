#pragma once
#include "IPostEffect.h"
#include <Render/Screen/Display.h>
#include <array>
#include <vector>

struct Bloom {
	uint32_t sampleCount = 16; // ブラーの比較ピクセル数
	uint32_t textureWidth; // Textureの幅
	uint32_t textureHeight; // Textureの高さ
	uint32_t maskTexture_;

	float strength = 0.8f;// 合成強度
};

class BloomEffect : public IPostEffect {
public:
	BloomEffect(uint32_t width = 1280, uint32_t height = 720) : width_(width), height_(height) {}

	void Initialize(SHEngine::Engine& engine) override;
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
	void Apply(PostEffectContext* context) override;

	void SetBloom(const Bloom& bloom);
	const Bloom& GetBloom() const { return bloom_; }

	void SetMaskTexture(int maskTexture) { bloom_.maskTexture_ = maskTexture; }

private:

	Bloom bloom_{};

	const uint32_t width_;
	const uint32_t height_;

	std::unique_ptr<SHEngine::Screen::Display> intermediate_;
	std::unique_ptr<SHEngine::GPUBuffer> indexBuffer_;
	std::unique_ptr<SHEngine::GPUBuffer> maskIndexBuffer_;

	std::unique_ptr<SHEngine::GPUBuffer> buffer_;
};
