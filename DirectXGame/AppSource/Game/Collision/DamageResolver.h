#pragma once
#include "CollisionEvent.h"
#include <Game/Enemy/EnemyManager.h>
#include <vector>

class PlayerManager;
class BulletManager;

/// @brief 衝突イベントからHP変更を一か所で解決する。HPの所有者以外が値を直接書き換えない。
/// 解決順:
///  1. 弾丸→敵の命中: 命中した各敵へダメージを与える。同じ弾丸が同フレームに複数の敵へ当たった場合は全てに与える。
///     敵のHPが0以下になっても、このフレーム中は残りの命中判定で有効な対象として扱う。
///  2. 敵→プレイヤーの接触: HPが残っている敵からの接触だけをプレイヤーへの被弾要求にする
///     (同フレームに撃破された敵からは被弾しない)。Playerは無敵中でなくスロットが空の場合に最初の1件だけ受け付ける。
///  3. プレイヤーの未処理の被弾要求を1回だけ適用し、全体無敵時間を開始する。
///  4. 最後に敵の死亡を確定する(撃破イベントは敵ごとに一度だけ)。
class DamageResolver {
public:

	struct Result {
		bool playerDamaged = false;
		std::vector<EnemyDefeatedEvent> defeatedEnemies;
	};

	/// @param 各Managerは非所有。DamageResolverより長寿命であること
	DamageResolver(PlayerManager* playerManager, EnemyManager* enemyManager, BulletManager* bulletManager);

	Result Resolve(const std::vector<CollisionEvent>& events);

private:

	PlayerManager* playerManager_ = nullptr;
	EnemyManager* enemyManager_ = nullptr;
	BulletManager* bulletManager_ = nullptr;
};
