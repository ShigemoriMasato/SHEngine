#pragma once
#include"Collision/Collider.h"

#include"OreUnitObject.h"
#include"Unit/MapChipField.h"

class OreUnit : public Collider {
public:

	// 状態
	enum class State {
		GoTo,   // 目的地に行く
		Return, // 帰宅
	};

public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="mapChipField">マップ機能</param>
	/// <param name="drawData">描画データ</param>
	/// <param name="apearPos">出現位置</param>
	/// <param name="targetPos">目標位置</param>
	void Initialize(MapChipField* mapChipField, DrawData drawData,const Vector3& apearPos,const Vector3& targetPos);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	void OnCollision(Collider* other) override {
		other;
	}

public:

	// 死亡フラグを取得
	bool IsDead()const { return isDead_; }

private:
	// マップデータ
	MapChipField* mapChipField_ = nullptr;

	// オブジェクトデータ
	std::unique_ptr<OreUnitObject> object_;

	int32_t hp_ = 0;

	bool isDead_ = false;

	// 移動ルート
	std::vector<Vector3> path_;

	// 家の場所
	Vector3 homePos_;

	// 移動速度
	float speed_ = 1.0f;

	// 状態
	State state_ = State::GoTo;

private:

	// 移動経路を求める
	void CalculatePath(const Vector3& goal);

	// 移動
	void Move();

};