#pragma once
#include "Player.h"
#include "PlayerConfig.h"
#include <memory>

class CollisionAdapter;
class Gun;
struct FieldConfig;

/// @brief Player の更新・設定・当たり判定登録をつなぐ。描画は PlayerRenderer が PlayerDrawState を使って行う。
/// 寿命: GunManager(提供されたGunの所有者)より短く、CollisionAdapterより短いこと。
class PlayerManager {
public:

	/// @param collision 当たり判定の境界(非所有)
	/// @param field フィールド設定(非所有)
	PlayerManager(CollisionAdapter* collision, const FieldConfig* field);
	~PlayerManager();
	PlayerManager(const PlayerManager&) = delete;
	PlayerManager& operator=(const PlayerManager&) = delete;

	/// @brief 設定を読み込み、フィールド中心にプレイヤーを生成する
	/// @param gun GunManagerから取得した銃(非所有)
	void Initialize(Gun* gun);

	/// @brief (2) 入力・移動の更新と当たり判定位置の同期
	void Update(float deltaTime, const PlayerInput& input);
	/// @brief (7) 射撃入力が有効ならGunへ射撃要求を出す
	void Shot(const PlayerInput& input, const Vector2& aimDirection);

	bool RequestDamage(int32_t amount);
	bool ApplyPendingDamage();

	Vector2 GetPosition() const { return player_->GetPosition(); }
	Vector3 GetWorldPosition() const;
	int32_t GetHp() const { return player_->GetHp(); }
	bool IsDead() const { return player_->IsDead(); }
	PlayerDrawState GetDrawState() const;

	const PlayerConfig& GetConfig() const { return config_; }
	void SetConfig(const PlayerConfig& config);
	void SaveConfig() const;

	static constexpr uint32_t kPlayerBodyID = 0;

private:

	CollisionAdapter* collision_ = nullptr;
	const FieldConfig* field_ = nullptr;

	PlayerConfig config_{};
	std::unique_ptr<Player> player_ = nullptr;
};
