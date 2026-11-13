#pragma once
#include "PlayerConfig.h"
#include "PlayerDrawState.h"
#include "PlayerInput.h"
#include <Utility/Vector.h>
#include <cstdint>

class Gun;
struct FieldConfig;

/// @brief プレイヤーの実行中状態(位置・速度・HP・無敵時間・未処理の被弾要求)を持つ更新クラス。
/// 描画系には依存せず、描画情報は PlayerDrawState として提供する。
/// 銃の具体型は知らず、Gun インターフェースへ射撃要求を出すだけ。
class Player {
public:

	/// @param field フィールド設定(非所有, GameScene所有でPlayerより長寿命)
	explicit Player(const FieldConfig* field);

	void Initialize(const PlayerConfig& config, const Vector2& position);
	/// @brief 移動パラメータ・色・無敵時間を差し替える(HPと半径は次プレイから)
	void SetConfig(const PlayerConfig& config);
	/// @param gun GunManager所有の銃(非所有)
	void SetGun(Gun* gun) { gun_ = gun; }

	/// @brief (2) 無敵時間を進め、入力に応じて移動する
	void Update(float deltaTime, const PlayerInput& input);

	/// @brief (7) 射撃入力が有効な間、毎フレーム Gun::Trigger を呼ぶ。銃の種類や弾丸の種類は判断しない
	void Shot(const PlayerInput& input, const Vector2& aimDirection);

	/// @brief 被弾要求。無敵時間中でなく、未処理の要求スロットが空の場合だけ記録する。記録したらtrue
	bool RequestDamage(int32_t amount);
	/// @brief 未処理の被弾要求を1回だけ適用してスロットを空にし、全体無敵時間を開始する。適用したらtrue
	bool ApplyPendingDamage();

	const Vector2& GetPosition() const { return position_; }
	const Vector2& GetPreviousPosition() const { return previousPosition_; }
	const Vector2& GetVelocity() const { return velocity_; }
	float GetRadius() const { return radius_; }
	int32_t GetHp() const { return hp_; }
	bool IsDead() const { return hp_ <= 0; }
	bool IsInvincible() const { return invincibleTimer_ > 0.0f; }

	PlayerDrawState GetDrawState(float floorHeight) const;

private:

	const FieldConfig* field_ = nullptr;
	Gun* gun_ = nullptr;

	PlayerConfig config_{};

	Vector2 position_ = {};
	Vector2 previousPosition_ = {};
	Vector2 velocity_ = {};
	float radius_ = 0.5f;

	int32_t hp_ = 0;
	float invincibleTimer_ = 0.0f;	// 全体無敵の残り時間(秒)

	// 未処理の被弾要求(同時に1件だけ)
	bool hasPendingDamage_ = false;
	int32_t pendingDamage_ = 0;
};
