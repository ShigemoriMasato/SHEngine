#pragma once
#include "Bullet.h"

/// @brief 発射方向へ直進するだけの基本弾
class SimpleBullet : public Bullet {
public:

	explicit SimpleBullet(uint32_t id);

	void Update(float deltaTime) override;
};
