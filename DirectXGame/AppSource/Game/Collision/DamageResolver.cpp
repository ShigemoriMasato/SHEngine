#include "DamageResolver.h"
#include <Game/Bullet/BulletManager.h>
#include <Game/Player/PlayerManager.h>

DamageResolver::DamageResolver(PlayerManager* playerManager, EnemyManager* enemyManager, BulletManager* bulletManager)
	: playerManager_(playerManager), enemyManager_(enemyManager), bulletManager_(bulletManager) {
}

DamageResolver::Result DamageResolver::Resolve(const std::vector<CollisionEvent>& events) {
	Result result;

	// 1. 弾丸 → 敵
	for (const CollisionEvent& event : events) {
		if (event.type != CollisionEventType::BulletEnemyHit) {
			continue;
		}

		Bullet* bullet = bulletManager_->FindBullet(event.sourceID);
		Enemy* enemy = enemyManager_->FindEnemy(event.enemyID);
		if (!bullet || !enemy || enemy->IsDead()) {
			continue;
		}

		enemy->ApplyDamage(bullet->GetDamage());
		// 消滅する弾丸でも、このフレーム中に既に検出された他の敵への命中は有効
		bullet->OnHit();
	}

	// 2. 敵 → プレイヤー(要求の受付可否はPlayerが判断する)
	for (const CollisionEvent& event : events) {
		if (event.type != CollisionEventType::PlayerEnemyContact) {
			continue;
		}

		const Enemy* enemy = enemyManager_->FindEnemy(event.enemyID);
		if (!enemy || enemy->IsDead() || !enemy->HasHp()) {
			continue;
		}

		playerManager_->RequestDamage(enemy->GetContactDamage());
	}

	// 3. 被弾要求を1回だけ適用
	result.playerDamaged = playerManager_->ApplyPendingDamage();

	// 4. 敵の死亡を確定
	result.defeatedEnemies = enemyManager_->ConfirmDeaths();

	return result;
}
