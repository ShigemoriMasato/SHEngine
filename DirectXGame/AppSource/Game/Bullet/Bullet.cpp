#include "Bullet.h"
#include "BulletDrawState.h"
#include <Game/Common/GameMath.h>

Bullet::Bullet(BulletType type, uint32_t id) : type_(type), id_(id) {
}

void Bullet::Initialize(const FireContext& context, const BulletConfig& config) {
	config_ = config;

	direction_ = { 0.0f, 1.0f };
	GameMath::TryNormalize(context.direction, direction_);

	position_ = context.origin;
	previousPosition_ = context.origin;
	velocity_ = direction_ * config_.speed;
	lifeTimer_ = config_.lifetime;
	isDead_ = false;
}

void Bullet::OnHit() {
	if (config_.destroyOnHit) {
		isDead_ = true;
	}
}

BulletCollisionInfo Bullet::GetCollisionInfo() const {
	BulletCollisionInfo info;
	info.previousPosition = previousPosition_;
	info.position = position_;
	info.radius = config_.radius;
	return info;
}

BulletDrawState Bullet::GetDrawState(float floorHeight) const {
	BulletDrawState state;
	state.type = type_;
	state.position = GameMath::ToWorld(position_, floorHeight + config_.height);
	state.direction = GameMath::ToWorld(direction_, 0.0f);
	state.visualRadius = config_.visualRadius;
	state.color = config_.color;
	return state;
}
