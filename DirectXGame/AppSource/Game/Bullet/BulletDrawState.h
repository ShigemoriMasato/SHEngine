#pragma once
#include "BulletType.h"
#include <Utility/Vector.h>

/// @brief 弾丸の描画スナップショット。BulletManagerが作り、BulletRendererへ値で渡す
struct BulletDrawState {
	BulletType type = BulletType::Simple;	// 描画種別(モデル選択に使う)
	Vector3 position = {};					// 中心(ワールド)
	Vector3 direction = { 0.0f, 0.0f, 1.0f };	// 進行方向(ワールド, 単位ベクトル)
	float visualRadius = 0.1f;				// 描画半径(ワールド単位)
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};
