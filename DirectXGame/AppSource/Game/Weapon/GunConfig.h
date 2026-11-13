#pragma once
#include <Game/Bullet/BulletConfig.h>
#include <Game/Bullet/BulletType.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief SimpleGunの設定。発射間隔・銃口オフセット・使用する弾丸の種類と設定だけを持つ。
/// 反映タイミング: 次プレイから(GunManager::Initialize で銃を生成する時点)
struct SimpleGunConfig {
	float fireInterval = 0.12f;					// 発射間隔(秒, 0.01～10)
	Vector2 muzzleOffset = { 0.0f, 0.7f };		// ローカル銃口オフセット(x: 照準の右方向, y: 照準方向, ワールド単位)
	BulletType bulletType = BulletType::Simple;	// 生成する弾丸の種類
	BulletConfig bullet{};						// 生成する弾丸の設定

	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);
	void Validate();
};

/// @brief 銃設定の保存単位。銃の種類が増えたらメンバを追加し、kVersionを上げる
struct GunConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "GunConfig.bin";

	SimpleGunConfig simple{};

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void Validate();
};
