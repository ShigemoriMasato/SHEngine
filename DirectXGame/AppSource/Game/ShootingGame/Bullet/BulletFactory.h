#pragma once
#include "Bullet.h"
#include <memory>

/// @brief 弾丸の種類IDから具象弾丸を生成する。新しい弾丸はCreateのswitchへ追加する
class BulletFactory {
public:

	/// @brief 未対応の種類の場合はnullptr
	std::unique_ptr<Bullet> Create(BulletType type, uint32_t id) const;
};
