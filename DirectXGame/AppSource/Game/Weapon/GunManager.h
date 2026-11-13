#pragma once
#include "Gun.h"
#include "GunConfig.h"
#include "GunType.h"
#include <array>
#include <memory>

class BulletManager;

/// @brief 銃の生成・所有・更新・設定の保存読み込みを担当する。
/// プレイヤーへは安定した寿命の Gun インターフェース(非所有ポインタ)を提供する。
/// 生成した銃はGunManagerの破棄まで解放しないため、PlayerManagerより長く生きること。
class GunManager {
public:

	/// @param bulletManager 弾丸生成の依頼先(非所有, GunManagerより長寿命)
	explicit GunManager(BulletManager* bulletManager);
	GunManager(const GunManager&) = delete;
	GunManager& operator=(const GunManager&) = delete;

	/// @brief 設定を読み込み、全種類の銃を生成する
	void Initialize();

	/// @brief (6) 各銃のクールダウン等を更新する
	void Update(float deltaTime);

	/// @brief 指定種類の銃。GunManagerの破棄まで有効。未生成ならnullptr
	Gun* GetGun(GunType type) const;

	void SaveConfig() const;
	const GunConfig& GetConfig() const { return config_; }

private:

	std::unique_ptr<Gun> CreateGun(GunType type) const;

	BulletManager* bulletManager_ = nullptr;
	GunConfig config_{};
	std::array<std::unique_ptr<Gun>, static_cast<size_t>(GunType::Count)> guns_{};
};
