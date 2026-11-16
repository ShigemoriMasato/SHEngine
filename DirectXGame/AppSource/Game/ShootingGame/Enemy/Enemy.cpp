#include "Enemy.h"
#include <Game/ShootingGame/Common/FieldConfig.h>
#include <Game/ShootingGame/Common/GameMath.h>

Enemy::Enemy(EnemyType type, uint32_t id, const FieldConfig* field)
	: field_(field), type_(type), id_(id) {
}

void Enemy::Initialize(const Vector2& spawnPosition) {
	position_ = spawnPosition;
	previousPosition_ = spawnPosition;
	velocity_ = { 0.0f, 0.0f };
	isDead_ = false;
}

EnemyDrawState Enemy::GetDrawState(float floorHeight) const {
	EnemyDrawState state;
	state.type = type_;
	state.id = id_;
	state.position = GameMath::ToWorld(position_, floorHeight + radius_);
	state.velocity = GameMath::ToWorld(velocity_, 0.0f);
	state.radius = radius_;
	return state;
}

void Enemy::ApplyDamage(float damage) {
	if (isDead_ || damage <= 0.0f) {
		return;
	}
	hp_ -= damage;
}

bool Enemy::ConfirmDeath() {
	if (isDead_ || HasHp()) {
		return false;
	}
	isDead_ = true;
	return true;
}

EnemyCollisionInfo Enemy::GetCollisionInfo() const {
	EnemyCollisionInfo info;
	info.previousPosition = previousPosition_;
	info.position = position_;
	info.radius = radius_;
	return info;
}

void Enemy::Move(float deltaTime, const Vector2& moveAmount, const ActorMotion::Param& param) {
	previousPosition_ = position_;
	ActorMotion::Step(position_, velocity_, moveAmount, param, deltaTime, *field_, radius_);
}
