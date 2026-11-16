#pragma once
#include <cstdint>

enum class CollisionEventType : uint32_t {
	PlayerEnemyContact,	// プレイヤーと敵の接触(sourceIDは未使用)
	BulletEnemyHit,		// 弾丸と敵の命中(sourceID = 弾丸ID)
};

/// @brief 衝突走査で得られた値型のイベント。Colliderのコールバック中には適用せず、走査後にDamageResolverが解決する
struct CollisionEvent {
	CollisionEventType type = CollisionEventType::PlayerEnemyContact;
	uint32_t sourceID = 0;	// 弾丸ID(BulletEnemyHitのみ)
	uint32_t enemyID = 0;	// 対象の敵ID
};
