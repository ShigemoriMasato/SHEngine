#pragma once
#include "IPostEffect.h"
#include <Render/Screen/Display.h>
#include <array>
#include <vector>

// Bloomの合成強度、ぼかし半径（縮小画像のピクセル単位）、縮小段数。
struct Bloom {
	uint32_t clientKernel; // ブラーの幅
	float sigma; // ガウス関数の標準偏差

	float strength = 1.0f;
	int maskTexture_;

	uint32_t iterations = 4;
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

	std::unique_ptr<SHEngine::Screen::Display> composite_;
	std::unique_ptr<SHEngine::GPUBuffer> compositeIndexBuffer_;
	std::unique_ptr<SHEngine::Screen::Display> intermediate_;
	std::unique_ptr<SHEngine::GPUBuffer> indexBuffer_;
	std::unique_ptr<SHEngine::GPUBuffer> maskIndexBuffer_;

	std::unique_ptr<SHEngine::GPUBuffer> buffer_;
};
