#include "Player.h"
#include <Game/ShootingGame/Common/ActorMotion.h>
#include <Game/ShootingGame/Common/FieldConfig.h>
#include <Game/ShootingGame/Common/GameMath.h>
#include <Game/ShootingGame/Weapon/Gun.h>
#include <algorithm>

Player::Player(const FieldConfig* field) : field_(field) {
}

void Player::Initialize(const PlayerConfig& config, const Vector2& position) {
	config_ = config;
	radius_ = config.radius;
	hp_ = config.maxHp;

	position_ = position;
	previousPosition_ = position;
	velocity_ = { 0.0f, 0.0f };

	invincibleTimer_ = 0.0f;
	hasPendingDamage_ = false;
	pendingDamage_ = 0;

	specialGauge_ = 0;
	isSpecialActivated_ = false;
}

void Player::SetConfig(const PlayerConfig& config) {
	config_ = config;
	// 上限が下がった場合に備えて丸める
	specialGauge_ = std::min(specialGauge_, config_.specialGaugeMax);
}

void Player::Update(float deltaTime, const PlayerInput& input) {
	invincibleTimer_ = std::max(0.0f, invincibleTimer_ - deltaTime);

	previousPosition_ = position_;
	isSpecialActivated_ = false;

	if (IsDead()) {
		return;
	}

	// ゲージが溜まりきった状態でKey::Specialが入力されたら発動し、ゲージを消費する
	if (input.special && IsSpecialGaugeFull()) {
		isSpecialActivated_ = true;
		specialGauge_ = 0;
	}

	ActorMotion::Param param;
	param.acceleration = config_.acceleration;
	param.damping = config_.damping;
	param.maxSpeed = config_.maxSpeed;
	ActorMotion::Step(position_, velocity_, input.move, param, deltaTime, *field_, radius_);
}

void Player::Shot(const PlayerInput& input, const Vector2& aimDirection) {
	if (!input.fire || !gun_ || IsDead()) {
		return;
	}

	FireContext context;
	context.origin = position_;
	if (!GameMath::TryNormalize(aimDirection, context.direction)) {
		return;
	}

	gun_->Trigger(context);
}

bool Player::RequestDamage(int32_t amount) {
	if (IsInvincible() || hasPendingDamage_ || IsDead() || amount <= 0) {
		return false;
	}

	hasPendingDamage_ = true;
	pendingDamage_ = amount;
	return true;
}

bool Player::ApplyPendingDamage() {
	if (!hasPendingDamage_) {
		return false;
	}

	hp_ = std::max(0, hp_ - pendingDamage_);
	hasPendingDamage_ = false;
	pendingDamage_ = 0;
	invincibleTimer_ = config_.invincibleTime;
	return true;
}

void Player::AddSpecialGauge(int32_t amount) {
	if (amount <= 0 || IsDead()) {
		return;
	}
	specialGauge_ = std::min(specialGauge_ + amount, config_.specialGaugeMax);
}

PlayerDrawState Player::GetDrawState(float floorHeight) const {
	PlayerDrawState state;
	state.position = GameMath::ToWorld(position_, floorHeight + radius_);
	state.velocity = GameMath::ToWorld(velocity_, 0.0f);
	state.radius = radius_;
	state.isInvincible = IsInvincible();
	state.isAlive = !IsDead();
	return state;
}
