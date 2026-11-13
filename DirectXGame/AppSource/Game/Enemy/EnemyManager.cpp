#include "EnemyManager.h"
#include <Game/Collision/CollisionAdapter.h>
#include <Game/Common/ConfigFile.h>
#include <Game/Common/FieldConfig.h>
#include <algorithm>
#include <cmath>
#include <numbers>

EnemyManager::EnemyManager(CollisionAdapter* collision, const FieldConfig* field)
	: collision_(collision), field_(field) {
}

EnemyManager::~EnemyManager() {
	// 当たり判定の登録を解除してから敵を破棄する
	for (const auto& enemy : enemies_) {
		collision_->Unregister(BodyKind::Enemy, enemy->GetID());
	}
}

void EnemyManager::Initialize() {
	ConfigFile::Load(EnemyConfig::kFileName, config_);
	spawnTimer_ = config_.spawn.initialDelay;
}

void EnemyManager::Update(float deltaTime, const EnemyTarget& target) {
	for (const auto& enemy : enemies_) {
		if (enemy->IsDead()) {
			continue;
		}

		enemy->Update(deltaTime, target);

		const EnemyCollisionInfo info = enemy->GetCollisionInfo();
		collision_->UpdateBody(BodyKind::Enemy, enemy->GetID(), info.previousPosition, info.position, info.radius);
	}

	// 更新ループの後で生成するため、反復中のコンテナは変更されない
	UpdateSpawn(deltaTime, target);
}

Enemy* EnemyManager::FindEnemy(uint32_t id) const {
	for (const auto& enemy : enemies_) {
		if (enemy->GetID() == id) {
			return enemy.get();
		}
	}
	return nullptr;
}

std::vector<EnemyDefeatedEvent> EnemyManager::ConfirmDeaths() {
	std::vector<EnemyDefeatedEvent> events;
	for (const auto& enemy : enemies_) {
		if (enemy->ConfirmDeath()) {
			// 死亡したフレームから衝突判定の対象外にする
			collision_->SetEnabled(BodyKind::Enemy, enemy->GetID(), false);

			EnemyDefeatedEvent event;
			event.id = enemy->GetID();
			event.type = enemy->GetType();
			event.position = enemy->GetPosition();
			events.push_back(event);
		}
	}
	return events;
}

void EnemyManager::RemoveDead() {
	// Colliderを先に解除してから、所有している敵を削除する
	for (const auto& enemy : enemies_) {
		if (enemy->IsDead()) {
			collision_->Unregister(BodyKind::Enemy, enemy->GetID());
		}
	}

	enemies_.erase(
		std::remove_if(enemies_.begin(), enemies_.end(), [](const std::unique_ptr<Enemy>& enemy) {
			return enemy->IsDead();
		}),
		enemies_.end());
}

void EnemyManager::CollectDrawStates(std::vector<EnemyDrawState>& out) const {
	out.clear();
	out.reserve(enemies_.size());
	for (const auto& enemy : enemies_) {
		if (enemy->IsDead()) {
			continue;
		}
		out.push_back(enemy->GetDrawState(field_->floorHeight));
	}
}

void EnemyManager::SaveConfig() const {
	ConfigFile::Save(EnemyConfig::kFileName, config_);
}

void EnemyManager::UpdateSpawn(float deltaTime, const EnemyTarget& target) {
	spawnTimer_ -= deltaTime;
	if (spawnTimer_ > 0.0f) {
		return;
	}

	const uint32_t maxCount = std::min(config_.spawn.maxCount, kMaxEnemyCount);
	if (enemies_.size() >= static_cast<size_t>(maxCount)) {
		// 空きが出たらすぐ出現できるよう、タイマーは0で待機する
		spawnTimer_ = 0.0f;
		return;
	}

	if (TrySpawn(EnemyType::Chaser, target.position)) {
		spawnTimer_ += config_.spawn.interval;
		// 大きなdeltaTimeでも1フレームに1体までにする
		spawnTimer_ = std::max(spawnTimer_, 0.0f);
	} else {
		// 条件を満たす位置が見つからなければ次フレームに再抽選する
		spawnTimer_ = 0.0f;
	}
}

bool EnemyManager::TrySpawn(EnemyType type, const Vector2& playerPosition) {
	std::unique_ptr<Enemy> enemy = factory_.Create(type, nextID_, config_, field_);
	if (!enemy) {
		return false;
	}

	Vector2 position;
	if (!TryFindSpawnPosition(playerPosition, enemy->GetRadius(), position)) {
		return false;
	}

	++nextID_;
	enemy->Initialize(position);

	const EnemyCollisionInfo info = enemy->GetCollisionInfo();
	collision_->Register(BodyKind::Enemy, enemy->GetID(), info.radius);
	collision_->UpdateBody(BodyKind::Enemy, enemy->GetID(), info.previousPosition, info.position, info.radius);

	enemies_.push_back(std::move(enemy));
	return true;
}

bool EnemyManager::TryFindSpawnPosition(const Vector2& playerPosition, float bodyRadius, Vector2& out) {
	const float movableRadius = FieldRule::GetMovableRadius(*field_, bodyRadius);
	const float minDistance = config_.spawn.minDistanceFromPlayer;

	for (uint32_t i = 0; i < config_.spawn.maxSpawnAttempts; ++i) {
		// 円内で一様になるよう半径はsqrtで抽選する
		const float radius = movableRadius * std::sqrt(unitDistribution_(randomEngine_));
		const float angle = unitDistribution_(randomEngine_) * 2.0f * std::numbers::pi_v<float>;
		const Vector2 candidate = field_->center + Vector2(std::cos(angle), std::sin(angle)) * radius;

		if ((candidate - playerPosition).Length() >= minDistance) {
			out = candidate;
			return true;
		}
	}
	return false;
}
