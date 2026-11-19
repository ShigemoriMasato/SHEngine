#include "SimpleBullet.h"

SimpleBullet::SimpleBullet(uint32_t id) : Bullet(BulletType::Simple, id) {
}

void SimpleBullet::Update(float deltaTime) {
	if (isDead_) {
		return;
	}

	previousPosition_ = position_;
	position_ += velocity_ * deltaTime;

	lifeTimer_ -= deltaTime;
	if (lifeTimer_ <= 0.0f) {
		isDead_ = true;
	}
}
