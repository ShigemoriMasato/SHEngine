#pragma once
#include <Utility/DataStructures.h>
#include <Tool/Binary/BinaryManager.h>

// SimplePostEffectに挿入するデータ
struct ISPEData {
	virtual void DrawImGui() = 0;
	virtual void Save(BinaryManager& bin) const = 0;
	virtual void Load(BinaryManager& bin) = 0;
};

struct Blur : public ISPEData {
	uint32_t kernelSize = 1;					// ブラーの幅（ピクセル単位）
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct GaussBlur : public ISPEData {
	uint32_t kernelSize = 3;					// ガウスカーネルのサイズ（奇数）
	float sigma = 1.0f;							// ガウス分布の標準偏差
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct Grayscale : public ISPEData {
	float intensity = 0.0f;						// グレースケール強度
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct Vignette : public ISPEData {
	Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };	// ビネットカラー
	float intensity;
	float radius;
	float softness;
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct Fade : public ISPEData {
	Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };		// フェードカラー
	float t = 1.0f; 								// フェードの進行度（0.0 - 1.0）
	Vector3 pad;
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct EdgeDetection : public ISPEData {
	uint32_t edgeTextureIndex = 0;				// エッジ検出用のテクスチャインデックス
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct Outline : public ISPEData {
	Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };	// アウトラインカラー
	uint32_t edgeTextureIndex = 0;				// エッジ検出用のテクスチャインデックス
	float strength = 0.0f;						// アウトラインの強さ
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct RadialBlur : public ISPEData {
	Vector2 center = { 0.5f, 0.5f };			// 中心座標（0.0 - 1.0）
	float strength = 0.01f;						// ブラーの強さ
	int sampleCount = 3;						// サンプル数
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};

struct Dissolve : public ISPEData {
	uint32_t noiseTextureIndex = 0;				// ノイズテクスチャのインデックス
	uint32_t transitionTextureIndex = 0;		// トランジションテクスチャのインデックス
	float threshold = 0.5f;						// ディゾルブのしきい値（0.0 - 1.0）
	float edgeThreshold = 0.1f;					// エッジのしきい値（0.0 - 1.0）
	Vector3 edgeColor = { 1.0f, 1.0f, 1.0f };	// エッジの色
	void DrawImGui() override;
	void Save(BinaryManager& bin) const override;
	void Load(BinaryManager& bin) override;
};
