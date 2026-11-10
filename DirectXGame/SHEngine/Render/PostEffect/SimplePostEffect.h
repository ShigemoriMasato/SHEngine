#pragma once
#include "IPostEffect.h"
#include "SimplePostEffectData.h"

class SimplePostEffect : public IPostEffect {
public:

	// @param data Shaderに送るCBVデータ
	// @param size 送るデータのサイズ
	// @param shaderName ピクセルシェーダーの名前 [PostEffect/]の先から入力。[.PS.hlsl]は省略してください。
	SimplePostEffect(std::unique_ptr<ISPEData> data, size_t size, const std::string& shaderName);

	void Initialize(SHEngine::Engine& engine) override;
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;

	//1フレームで何回も使うときは、その数だけSimplePostEffectを作成することを推奨する。
	void Apply(PostEffectContext* context) override;

private:

	std::unique_ptr<ISPEData> data_ = nullptr;
	size_t size_ = 0;
	std::unique_ptr<SHEngine::GPUBuffer> buffer_;
	std::string shaderPath_;
};
