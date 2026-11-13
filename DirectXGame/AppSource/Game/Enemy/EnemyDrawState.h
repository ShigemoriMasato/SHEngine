#pragma once
#include "EnemyType.h"
#include <Utility/Vector.h>

/// @brief 敵の描画スナップショット。EnemyManagerが作り、EnemyRendererへ値で渡す
struct EnemyDrawState {
	EnemyType type = EnemyType::Chaser;			// 描画種別(モデル選択に使う)
	Vector3 position = {};						// 中心(ワールド)
	Vector3 forward = { 0.0f, 0.0f, 1.0f };		// 向き(ワールド, 単位ベクトル)
	float radius = 0.5f;						// 描画半径(ワールド単位)
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};
