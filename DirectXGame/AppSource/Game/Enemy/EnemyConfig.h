#pragma once
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief ChaserEnemyの調整値。反映タイミング: 次に出現する敵から
struct ChaserEnemyConfig {
	float acceleration = 20.0f;		// 追跡加速度(ワールド単位/秒^2, 0～1000)
	float damping = 0.3f;			// 1秒あたりの速度残存率(0.0001～1)
	float maxSpeed = 4.0f;			// 最大速度(ワールド単位/秒, 0.01～1000)
	float radius = 0.5f;			// 当たり判定・描画半径(ワールド単位, 0.05～10)
	float maxHp = 3.0f;				// 初期HP(0.01～100000)
	int32_t contactDamage = 1;		// 接触時にプレイヤーへ要求するダメージ量(0～100000)
	Vector4 color = { 1.0f, 0.35f, 0.3f, 1.0f };	// 描画色(RGBA 0～1)

	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);
	void Validate();
};

/// @brief 敵の出現設定。反映タイミング: 次プレイから
struct EnemySpawnConfig {
	float initialDelay = 1.0f;				// ゲーム開始から最初の出現までの時間(秒, 0～600)
	float interval = 1.2f;					// 出現間隔(秒, 0.05～600)
	uint32_t maxCount = 24;					// 最大同時数(1～256)
	float minDistanceFromPlayer = 6.0f;		// プレイヤーからの最小出現距離(ワールド単位, 0～1000)
	uint32_t maxSpawnAttempts = 16;			// 出現位置の抽選回数。全て最小距離未満なら次フレームで再抽選(1～64)

	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);
	void Validate();
};

/// @brief 敵設定の保存単位。敵の種類が増えたらメンバを追加し、kVersionを上げる
struct EnemyConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "EnemyConfig.bin";

	ChaserEnemyConfig chaser{};
	EnemySpawnConfig spawn{};

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void Validate();
};
