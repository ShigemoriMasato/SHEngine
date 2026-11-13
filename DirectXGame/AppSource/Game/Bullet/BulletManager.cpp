#include "BulletManager.h"
#include <Game/Collision/CollisionAdapter.h>
#include <Game/Common/FieldConfig.h>
#include <algorithm>

BulletManager::BulletManager(CollisionAdapter* collision, const FieldConfig* field)
	: collision_(collision), field_(field) {
}

BulletManager::~BulletManager() {
	// 当たり判定の登録を解除してから弾丸を破棄する
	for (const auto& bullet : bullets_) {
		collision_->Unregister(BodyKind::Bullet, bullet->GetID());
	}
}

void BulletManager::Spawn(BulletType type, const BulletConfig& config, const FireContext& context) {
	if (bullets_.size() + pendingBullets_.size() >= kMaxBulletCount) {
		return;
	}

	std::unique_ptr<Bullet> bullet = factory_.Create(type, nextID_);
	if (!bullet) {
		return;
	}
	++nextID_;

	bullet->Initialize(context, config);
	pendingBullets_.push_back(std::move(bullet));
}

void BulletManager::Update(float deltaTime) {
	for (const auto& bullet : bullets_) {
		if (bullet->IsDead()) {
			continue;
		}

		bullet->Update(deltaTime);

		// フィールド外へ出た弾丸は死亡扱い
		if (FieldRule::IsOutside(*field_, bullet->GetPosition(), bullet->GetRadius())) {
			bullet->Kill();
		}

		const BulletCollisionInfo info = bullet->GetCollisionInfo();
		collision_->UpdateBody(BodyKind::Bullet, bullet->GetID(), info.previousPosition, info.position, info.radius);

		// 寿命切れ等で死亡した弾丸は、このフレームの衝突判定から外す
		if (bullet->IsDead()) {
			collision_->SetEnabled(BodyKind::Bullet, bullet->GetID(), false);
		}
	}

	// このフレームに生成された弾丸を管理対象へ加える(更新は次フレームから)
	for (auto& bullet : pendingBullets_) {
		const BulletCollisionInfo info = bullet->GetCollisionInfo();
		collision_->Register(BodyKind::Bullet, bullet->GetID(), info.radius);
		collision_->UpdateBody(BodyKind::Bullet, bullet->GetID(), info.previousPosition, info.position, info.radius);
		bullets_.push_back(std::move(bullet));
	}
	pendingBullets_.clear();
}

void BulletManager::RemoveDead() {
	// Colliderを先に解除してから、所有している弾丸を削除する
	for (const auto& bullet : bullets_) {
		if (bullet->IsDead()) {
			collision_->Unregister(BodyKind::Bullet, bullet->GetID());
		}
	}

	bullets_.erase(
		std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& bullet) {
			return bullet->IsDead();
		}),
		bullets_.end());
}

Bullet* BulletManager::FindBullet(uint32_t id) const {
	for (const auto& bullet : bullets_) {
		if (bullet->GetID() == id) {
			return bullet.get();
		}
	}
	return nullptr;
}

void BulletManager::CollectDrawStates(std::vector<BulletDrawState>& out) const {
	out.clear();
	out.reserve(bullets_.size());
	for (const auto& bullet : bullets_) {
		if (bullet->IsDead()) {
			continue;
		}
		out.push_back(bullet->GetDrawState(field_->floorHeight));
	}
}
