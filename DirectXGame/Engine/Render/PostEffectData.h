#pragma once
#include <Utility/Vector.h>

struct Blur {
	uint32_t kernelSize = 1;					// ブラーの幅（ピクセル単位）
};

struct GaussBlur {
	uint32_t kernelSize = 3;					// ガウスカーネルのサイズ（奇数）
	float sigma = 1.0f;						// ガウス分布の標準偏差
};

struct Grayscale {
	float intensity = 0.0f; // グレースケール強度
};

struct Vignette {
	Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };	// ビネットカラー
	float intensity;
	float radius;
	float softness;
};

struct Fade {
	Vector3 color = { 0.0f, 0.0f, 0.0f };	// フェードカラー（White時は{1,1,1}）
	float alpha = 0.0f;						// フェードの透明度 (0.0 - 1.0)
};

struct EdgeDetection {
	uint32_t edgeTextureIndex = 0;				// エッジ検出用のテクスチャインデックス
};

struct Outline {
	Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };	// アウトラインカラー
	uint32_t edgeTextureIndex = 0;				// エッジ検出用のテクスチャインデックス
	float strength = 0.0f;						// アウトラインの強さ
};

struct RadialBlur {
	Vector2 center = { 0.5f, 0.5f };			// 中心座標（0.0 - 1.0）
	float strength = 0.01f;						// ブラーの強さ
	int sampleCount = 3;						// サンプル数
};
