#pragma once
#include <optional>
#include<array>
#include<functional>
#include<numbers>

#include"Collision/Collider.h"

#include"OreUnitObject.h"
#include"Unit/MapChipField.h"

class OreUnit : public Collider {
public:

	// 状態
	enum class State {
		GoTo,   // 目的地に行く
		Mining, // 採掘
		ToDeliver, // 納品
		Return, // 帰宅

		MaxCount // 数
	};

	// 帰宅する時の動き
	enum class ReturnPhase {
		Rise,     // 上昇
		Move,     // 移動
		Fall,     // 落下
	};

public:

	OreUnit(MapChipField* mapChipField, DrawData drawData, Vector3* playerPos);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mapChipField">マップ機能</param>
	/// <param name="drawData">描画データ</param>
	/// <param name="apearPos">出現位置</param>
	/// <param name="targetPos">目標位置</param>
	void Init(const Vector3& apearPos,const Vector3& targetPos);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	// 当たり判定
	void OnCollision(Collider* other) override;

public:

	// 死亡フラグを取得
	bool IsDead()const { return isDead_; }

	// 有効フラグ
	bool IsActive() const { return isActive_; }

private:
	// マップデータ
	MapChipField* mapChipField_ = nullptr;

	// オブジェクトデータ
	std::unique_ptr<OreUnitObject> object_;

	// 状態
	State state_ = State::GoTo;
	// 状態の変更を管理
	std::optional<State> stateRequest_ = std::nullopt;
	// 状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(State::MaxCount)> statesTable_;
	// 指定した状態をおこなうためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(State::MaxCount)> resetStatesTable_;

	// 死亡フラグ
	bool isDead_ = false;
	// 有効フラグ
	bool isActive_ = false;

	// 体力
	int32_t hp_ = 0;

	// 移動ルート
	std::vector<Vector3> path_;
	// 家の場所
	Vector3 homePos_ = {};
	// プレイヤーの位置
	Vector3* playerPos_ = nullptr;
	// 回転する移動先
	Vector3 toRotPos_ = {};

	// 体力の計算に使用
	float lifeTimer_ = 0.0f;

	float timer_ = 0.0f;

	// 円の当たり判定
	Circle circleCollider_;

	bool isHit = false;

	// 鉱石からの離脱を取得
	bool isRemoveOre_ = false;

	// 拠点に戻るとのフェーズ
	ReturnPhase returnPhase_ = ReturnPhase::Fall;
	Vector3 startMovePos;
	Vector3 endMovePos;

	// デバック用
	std::string kGroupName_ = "OreUnit";

private: // 調整項目

	// ダメージを食らう間隔
	float damageTime_ = 1.0f;

	// 最大体力
	int32_t maxHp_ = 10;

	// 移動速度
	float moveSpeed_ = 2.0f;

	// 角度補間
	float rotateSpeed_ = 3.0f;

	// 採掘時間
	float miningTime_ = 1.0f;

	// 高さ
	float risePosY_ = 5.0f;

	// 帰宅の時間
	float riseTime_ = 1.0f;
	float moveTime_ = 1.0f;
	float FallTime_ = 1.0f;

private:

	// 移動経路を求める
	void CalculatePath(const Vector3& goal);

	// 移動
	void Move();

	// 進行方向に回転する
	void Rotate();

	// 行きの更新処理
	void GoToUpdate();

	// 採掘の更新処理
	void MiningUpdate();

	// 納品の更新処理
	void ToDeliverUpdate();

	// 返りの更新処理
	void ReturnUpdate();

private:
	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};

// ヘルプ関数
namespace {

	// 最短角度を求める
	float GetShortAngleY(float diffY) {
		while (diffY > std::numbers::pi_v<float>) {
			diffY -= std::numbers::pi_v<float> *2.0f;
		}
		while (diffY < -std::numbers::pi_v<float>) {
			diffY += std::numbers::pi_v<float> *2.0f;
		}
		return diffY;
	}
}