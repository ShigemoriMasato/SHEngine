#pragma once
#include <Utility/Vector.h>

struct Blur {
	uint32_t blurWidth = 1;					// ブラーの幅（ピクセル単位）
	uint32_t blurHeight = 1;					// ブラーの高さ（ピクセル単位）
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

struct GridTransition {
	float progress = 0.0f;					// 遷移の進行度 (0.0 - 1.0)
	float gridSize = 16.0f;					// グリッドのサイズ（1辺のタイル数）
	float fadeColor = 0.0f;					// フェード色 (0=黒, 1=白)
	float pattern = 0.0f;					// パターン (0=波紋状, 1=ランダム, 2=左から右, 3=上から下, 4=チェッカーボード)
};

struct SlowMotion {
	float chromaticAberration = 0.0f;		// 色収差強度 (0.0 - 1.0)
	float vignetteStrength = 0.0f;			// ビネット強度 (0.0 - 1.0)
	float saturation = 1.0f;				// 彩度 (0.0=モノクロ, 1.0=通常)
	float intensity = 0.0f;					// 全体の強度 (0.0 - 1.0)
};

struct Glitch {
	float intensity = 0.0f;					// 全体の強度 (0.0 - 1.0)
	float rgbSplit = 0.0f;					// RGB色収差の強度 (0.0 - 1.0)
	float scanlineIntensity = 0.0f;			// スキャンライン強度 (0.0 - 1.0)
	float blockIntensity = 0.0f;			// ブロックノイズ強度 (0.0 - 1.0)
	float time = 0.0f;						// 時間パラメータ（アニメーション用
};
