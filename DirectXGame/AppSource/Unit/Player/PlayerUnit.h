#pragma once
#include"Unit/MapChipField.h"
#include"PlayerUnitObject.h"
#include"Common/KeyConfig/KeyManager.h"

class PlayerUnit {
public:

	void Initialize(MapChipField* mapChipField, DrawData drawData,const Vector3& pos, KeyManager* keyManager);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

private:

	// マップデータ
	MapChipField* mapChipField_ = nullptr;

	// オブジェクトデータ
	std::unique_ptr<PlayerUnitObject> object_;

	// キーを取得する
	KeyManager* keyManager_ = nullptr;

	// 体力
	int32_t hp_ = 0;

	// サイズ
	Vector3 size_ = { 1.0f,1.0f,1.0f };
	
	// 速度 
	Vector3 velocity_ = {};

	// 速さ
	float speed_ = 20.0f;

private:

	// 入力処理
	void ProcessMoveInput();

	/// <summary>
	/// マップの衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckMapCollision(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 上方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionAbove(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 下方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionBelow(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 右方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionRight(MapChipField::CollisionMapInfo& info);

	/// <summary>
	/// 左方向の衝突判定
	/// </summary>
	/// <param name="info"></param>
	void CheckCollisionLeft(MapChipField::CollisionMapInfo& info);

};