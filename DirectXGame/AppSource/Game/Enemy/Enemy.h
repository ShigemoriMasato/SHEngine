#pragma once
#include "EnemyDrawState.h"
#include "EnemyTarget.h"
#include "EnemyType.h"
#include <Game/Common/ActorMotion.h>
#include <Utility/Vector.h>
#include <cstdint>

struct FieldConfig;

/// @brief 接触判定に使う読み取り専用情報
struct EnemyCollisionInfo {
	Vector2 previousPosition = {};	// 前フレームの位置(XZ)
	Vector2 position = {};			// 現在位置(XZ)
	float radius = 0.0f;
};

/// @brief 敵の共通インターフェース兼共通基底クラス。EnemyManagerはこの型を通して敵を管理する。
/// 座標・速度・HP・当たり判定半径・死亡フラグなど共通の状態を持ち、固有の移動・攻撃・外観は派生クラスで実装する。
/// Update は状態更新だけを行い、描画やプレイヤーのHP変更は行わない(接触ダメージは衝突イベント経由で上位が適用する)。
class Enemy {
public:

	/// @param field フィールド設定(非所有, GameScene所有で敵より長寿命)
	Enemy(EnemyType type, uint32_t id, const FieldConfig* field);
	virtual ~Enemy() = default;
	Enemy(const Enemy&) = delete;
	Enemy& operator=(const Enemy&) = delete;

	/// @brief 出現位置で共通状態を初期化する。HP・半径・色は派生クラスのコンストラクタで設定済みであること
	virtual void Initialize(const Vector2& spawnPosition);

	/// @brief 状態を更新する(描画は行わない)
	/// @param target 追跡対象の読み取り専用情報
	virtual void Update(float deltaTime, const EnemyTarget& target) = 0;

	/// @brief 接触時にプレイヤーへ要求するダメージ量
	virtual int32_t GetContactDamage() const = 0;

	/// @brief 描画スナップショット(高さはフィールドの床高さを加えて求める)
	virtual EnemyDrawState GetDrawState(float floorHeight) const;

	/// @brief HPを減らす。死亡の確定は ConfirmDeath で衝突解決の最後に行う
	void ApplyDamage(float damage);
	/// @brief HPが0以下なら死亡を確定する。確定したフレームだけtrue(撃破イベントを一度だけ出すため)
	bool ConfirmDeath();

	bool IsDead() const { return isDead_; }
	bool HasHp() const { return hp_ > 0.0f; }
	uint32_t GetID() const { return id_; }
	EnemyType GetType() const { return type_; }
	const Vector2& GetPosition() const { return position_; }
	float GetRadius() const { return radius_; }
	EnemyCollisionInfo GetCollisionInfo() const;

protected:

	/// @brief 加速度・減衰・最大速度を使った共通の移動処理
	/// @param moveAmount 加速度の向きと量(長さ0～1)
	void Move(float deltaTime, const Vector2& moveAmount, const ActorMotion::Param& param);

	const FieldConfig* field_ = nullptr;

	Vector2 position_ = {};
	Vector2 previousPosition_ = {};
	Vector2 velocity_ = {};
	Vector2 forward_ = { 0.0f, 1.0f };

	float hp_ = 1.0f;
	float radius_ = 0.5f;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool isDead_ = false;

private:

	EnemyType type_;
	uint32_t id_;
};
