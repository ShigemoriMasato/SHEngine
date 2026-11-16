#pragma once
#include "EnemyType.h"
#include <Utility/Vector.h>
#include <cstdint>

/// @brief 敵の描画スナップショット。EnemyManagerが作り、EnemyRendererへ値で渡す
struct EnemyDrawState {
	EnemyType type = EnemyType::Chaser;			// 描画種別(モデル選択に使う)
	Vector3 position = {};						// 中心(ワールド)
	uint32_t id = 0;							// 個体ID(描画側の転がり回転の対応付けに使う)
	Vector3 velocity = {};						// 転がり表現用の速度(ワールド, Y=0)
	float radius = 0.5f;						// 描画半径(ワールド単位)
};
