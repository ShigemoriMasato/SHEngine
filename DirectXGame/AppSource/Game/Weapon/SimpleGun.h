#pragma once
#include "Gun.h"
#include "GunConfig.h"

class BulletManager;

/// @brief 発射間隔のクールダウンだけを管理するシンプルな銃。
/// 装弾数・弾薬・リロード・武器切替は持たない。生成した弾丸は保持・追跡しない。
class SimpleGun : public Gun {
public:

	/// @param bulletManager 弾丸生成の依頼先(非所有, GunManagerより長寿命)
	SimpleGun(const SimpleGunConfig& config, BulletManager* bulletManager);

	void Update(float deltaTime) override;
	void Trigger(const FireContext& context) override;

private:

	SimpleGunConfig config_;
	BulletManager* bulletManager_ = nullptr;

	// 次に発射できるまでの残り時間(秒)。0以下で発射可能
	float cooldown_ = 0.0f;
};
