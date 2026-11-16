#pragma once
#include <Utility/Vector.h>

/// @brief 敵の更新に渡す追跡対象の読み取り専用情報。Playerクラス全体への依存を避けるための値型
struct EnemyTarget {
	Vector2 position = { 0.0f, 0.0f };	// 追跡対象の位置(XZ)
	Vector2 velocity = { 0.0f, 0.0f };	// 追跡対象の速度(XZ)。予測追跡などの将来の敵用
};
