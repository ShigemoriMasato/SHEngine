#pragma once
#include <cstdint>

/// @brief 敵の種類ID。新しい敵はここへ追加し、EnemyFactory と EnemyRenderer のモデル選択へ追加する
enum class EnemyType : uint32_t {
	Chaser,

	Count
};
