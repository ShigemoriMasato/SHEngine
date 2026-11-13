#pragma once
#include "Bullet.h"
#include "BulletDrawState.h"
#include "BulletFactory.h"
#include <memory>
#include <vector>

class CollisionAdapter;
struct FieldConfig;

/// @brief 弾丸の生成・所有・更新・死亡判定・削除を一元的に担当する。
/// 銃からは種類・設定・発射コンテキストを受け取るだけで、弾丸のポインタは返さない。
/// 寿命: GunManager(所有するGunが参照する)より長く生きること。
class BulletManager {
public:

	/// @param collision 当たり判定の境界(非所有, GameScene所有でBulletManagerより長寿命)
	/// @param field フィールド設定(非所有, GameScene所有)
	BulletManager(CollisionAdapter* collision, const FieldConfig* field);
	~BulletManager();
	BulletManager(const BulletManager&) = delete;
	BulletManager& operator=(const BulletManager&) = delete;

	/// @brief 弾丸の生成依頼。生成した弾丸は次フレームのUpdateから更新される
	void Spawn(BulletType type, const BulletConfig& config, const FireContext& context);

	/// @brief (8) 既存の弾丸を更新し、寿命切れ・フィールド外の弾丸を死亡扱いにする。
	/// 最後に、このフレームに生成された弾丸を管理対象へ加える(当たり判定は生成位置で行う)
	void Update(float deltaTime);

	/// @brief (10) 死亡した弾丸のColliderを解除してから削除する
	void RemoveDead();

	/// @brief 衝突解決用。RemoveDeadまで有効な非所有ポインタ。見つからなければnullptr
	Bullet* FindBullet(uint32_t id) const;

	void CollectDrawStates(std::vector<BulletDrawState>& out) const;
	size_t GetBulletCount() const { return bullets_.size(); }

	// 同時に存在できる弾丸の上限(描画インスタンス数の上限と揃える)
	static constexpr size_t kMaxBulletCount = 512;

private:

	CollisionAdapter* collision_ = nullptr;
	const FieldConfig* field_ = nullptr;

	BulletFactory factory_;
	std::vector<std::unique_ptr<Bullet>> bullets_;
	// このフレームに生成され、まだ更新されていない弾丸
	std::vector<std::unique_ptr<Bullet>> pendingBullets_;

	uint32_t nextID_ = 1;
};
