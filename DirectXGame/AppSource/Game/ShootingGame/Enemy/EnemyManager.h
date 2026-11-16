#pragma once
#include "Enemy.h"
#include "EnemyConfig.h"
#include "EnemyDrawState.h"
#include "EnemyFactory.h"
#include <memory>
#include <random>
#include <vector>

class CollisionAdapter;
struct FieldConfig;

/// @brief 撃破イベント。死亡が確定したフレームに一度だけ出る(初期実装ではスコア等に使用しない)
struct EnemyDefeatedEvent {
	uint32_t id = 0;
	EnemyType type = EnemyType::Chaser;
	Vector2 position = {};
};

/// @brief 敵の生成・所有・更新・死亡判定・削除を担当する。敵は Enemy 基底クラスとして所有する。
/// 描画は行わず、EnemyRenderer へは EnemyDrawState を渡す。
class EnemyManager {
public:

	/// @param collision 当たり判定の境界(非所有, EnemyManagerより長寿命)
	/// @param field フィールド設定(非所有)
	EnemyManager(CollisionAdapter* collision, const FieldConfig* field);
	~EnemyManager();
	EnemyManager(const EnemyManager&) = delete;
	EnemyManager& operator=(const EnemyManager&) = delete;

	/// @brief 設定を読み込み、出現タイマーを初期化する
	void Initialize();

	/// @brief (5) 既存の敵を更新し、その後に時間経過で新しい敵を生成する(新しい敵は次フレームから更新)
	void Update(float deltaTime, const EnemyTarget& target);

	/// @brief 衝突解決用。RemoveDeadまで有効な非所有ポインタ。見つからなければnullptr
	Enemy* FindEnemy(uint32_t id) const;

	/// @brief (9の最後) HPが0以下の敵の死亡を確定し、撃破イベントを返す。死亡処理は敵ごとに一度だけ
	std::vector<EnemyDefeatedEvent> ConfirmDeaths();

	/// @brief (10) 死亡した敵のColliderを解除してから削除する
	void RemoveDead();

	void CollectDrawStates(std::vector<EnemyDrawState>& out) const;
	size_t GetEnemyCount() const { return enemies_.size(); }

	const EnemyConfig& GetConfig() const { return config_; }
	void SaveConfig() const;

	// 同時に存在できる敵の上限(描画インスタンス数の上限と揃える)
	static constexpr uint32_t kMaxEnemyCount = 256;

private:

	void UpdateSpawn(float deltaTime, const EnemyTarget& target);
	bool TrySpawn(EnemyType type, const Vector2& playerPosition);
	bool TryFindSpawnPosition(const Vector2& playerPosition, float bodyRadius, Vector2& out);

	CollisionAdapter* collision_ = nullptr;
	const FieldConfig* field_ = nullptr;

	EnemyConfig config_{};
	EnemyFactory factory_;
	std::vector<std::unique_ptr<Enemy>> enemies_;

	float spawnTimer_ = 0.0f;	// 次の出現までの残り時間(秒)
	uint32_t nextID_ = 1;

	std::mt19937 randomEngine_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> unitDistribution_ = std::uniform_real_distribution<float>(0.0f, 1.0f);
};
