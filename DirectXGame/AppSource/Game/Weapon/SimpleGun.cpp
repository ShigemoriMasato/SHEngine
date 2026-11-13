#include "SimpleGun.h"
#include <Game/Bullet/BulletManager.h>
#include <Game/Common/GameMath.h>
#include <algorithm>

SimpleGun::SimpleGun(const SimpleGunConfig& config, BulletManager* bulletManager)
	: config_(config), bulletManager_(bulletManager) {
}

void SimpleGun::Update(float deltaTime) {
	cooldown_ = std::max(0.0f, cooldown_ - deltaTime);
}

void SimpleGun::Trigger(const FireContext& context) {
	// クールダウン中から押し続けていても、発射可能になったフレームで撃つ
	if (cooldown_ > 0.0f || !bulletManager_) {
		return;
	}

	Vector2 forward;
	if (!GameMath::TryNormalize(context.direction, forward)) {
		return;
	}

	// ローカル銃口オフセット(x: 右, y: 前)をワールド位置へ変換する
	const Vector2 right = { forward.y, -forward.x };
	FireContext muzzle;
	muzzle.origin = context.origin + right * config_.muzzleOffset.x + forward * config_.muzzleOffset.y;
	muzzle.direction = forward;

	bulletManager_->Spawn(config_.bulletType, config_.bullet, muzzle);
	cooldown_ = config_.fireInterval;
}
