#pragma once
#include <Utility/Vector.h>

struct Blur {
	float intensity = 0.0f; // ブラー強度
	float kernelSize = 0.0f; // カーネルサイズ
	int radius = 0;
	float pad;
};

struct HeavyBlur {
	float intensity = 0.0f; // ヘビーブラー強度
	float kernelSize = 1;         // ブラーのパス数
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

struct ConstantScanline {
	float intensity = 0.3f;					// 走査線の強度 (0.0 - 1.0)
	float speed = 1.0f;						// 走査線の移動速度
	float lineWidth = 0.002f;				// 走査線の幅
	float time = 0.0f;						// 時間パラメータ（アニメーション用）
};
