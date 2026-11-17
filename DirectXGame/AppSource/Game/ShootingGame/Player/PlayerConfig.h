#pragma once
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief プレイヤーの調整値。実行中状態(HP現在値・位置・速度・無敵残り時間)は含めない
struct PlayerConfig {
	static constexpr uint32_t kVersion = 2;	// 2: 描画色を削除(色はモデルのマテリアル準拠)
	static constexpr uint32_t kLegacyColorVersion = 1;	// 描画色を含む旧形式
	static inline const std::string kFileName = "PlayerConfig.bin";

	float acceleration = 40.0f;		// 入力量1.0のときの加速度(ワールド単位/秒^2, 0～1000)。即時反映
	float damping = 0.05f;			// 1秒あたりの速度残存率(0.0001～1。1で減衰なし)。即時反映
	float maxSpeed = 8.0f;			// 最大速度(ワールド単位/秒, 0.01～1000)。即時反映
	static inline const float radius = 0.5f;		// 当たり判定・描画半径(ワールド単位, 0.05～10)。次プレイから
	int32_t maxHp = 30;				// 初期HP(1～100000)。次プレイから
	float invincibleTime = 1.0f;	// 被弾後の全体無敵時間(秒, 0～60)。次の被弾から
	int32_t specialGaugeMax = 10;	// 必殺技ゲージの上限(撃破数, 1～10000)。即時反映

	bool isInvincible = false;	// 無敵状態(デバッグ用)。即時反映

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void DrawImGui();
	void Validate();
};
