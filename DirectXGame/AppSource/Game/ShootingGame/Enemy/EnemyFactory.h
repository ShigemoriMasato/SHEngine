#pragma once
#include "Enemy.h"
#include "EnemyConfig.h"
#include <memory>

/// @brief 敵の種類IDと設定から具象敵を生成する。新しい敵はCreateのswitchへ追加する
class EnemyFactory {
public:

	/// @brief 未対応の種類の場合はnullptr
	/// @param field フィールド設定(非所有)
	std::unique_ptr<Enemy> Create(EnemyType type, uint32_t id, const EnemyConfig& config, const FieldConfig* field) const;
};
