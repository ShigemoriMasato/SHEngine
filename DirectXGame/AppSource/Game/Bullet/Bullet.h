#pragma once
#include "BulletType.h"
#include "BulletConfig.h"
#include "BulletDrawState.h"
#include <Game/Weapon/FireContext.h>
#include <Utility/Vector.h>
#include <cstdint>

/// @brief 当たり判定用の読み取り専用情報。前フレーム位置を含め、移動軌跡での判定に使う
struct BulletCollisionInfo {
	Vector2 previousPosition = {};	// 前フレームの位置(XZ)
	Vector2 position = {};			// 現在位置(XZ)
	float radius = 0.0f;
};

/// @brief 弾丸の基底クラス。BulletManagerが所有し、銃やプレイヤーはポインタを受け取らない。
/// Update は自身の状態更新だけを行い、描画は BulletRenderer が描画スナップショットを使って行う。
class Bullet {
public:

	Bullet(BulletType type, uint32_t id);
	virtual ~Bullet() = default;
	Bullet(const Bullet&) = delete;
	Bullet& operator=(const Bullet&) = delete;

	/// @brief 発射コンテキスト(銃口位置・方向)と弾丸設定で初期化する
	virtual void Initialize(const FireContext& context, const BulletConfig& config);

	/// @brief 時間経過に応じて自身の状態を更新する(秒)
	virtual void Update(float deltaTime) = 0;

	/// @brief 敵に命中したことを通知する。消滅するか貫通するかは設定・種類ごとに決める
	virtual void OnHit();

	/// @brief 寿命切れ・フィールド外など、外部要因で破棄状態にする
	void Kill() { isDead_ = true; }

	bool IsDead() const { return isDead_; }
	uint32_t GetID() const { return id_; }
	BulletType GetType() const { return type_; }
	float GetDamage() const { return config_.damage; }
	const Vector2& GetPosition() const { return position_; }
	float GetRadius() const { return config_.radius; }

	BulletCollisionInfo GetCollisionInfo() const;
	/// @brief 描画スナップショット(高さはフィールドの床高さを加えて求める)
	virtual BulletDrawState GetDrawState(float floorHeight) const;

protected:

	BulletConfig config_{};
	Vector2 position_ = {};
	Vector2 previousPosition_ = {};
	Vector2 velocity_ = {};
	Vector2 direction_ = { 0.0f, 1.0f };
	float lifeTimer_ = 0.0f;	// 残り寿命(秒)
	bool isDead_ = false;

private:

	BulletType type_;
	uint32_t id_;
};
