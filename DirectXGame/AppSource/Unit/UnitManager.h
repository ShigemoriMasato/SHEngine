#pragma once
#include<unordered_map>
#include <deque>
#include"Common/KeyConfig/KeyManager.h"

#include"Unit/Player/PlayerUnit.h"
#include"Unit/Ore/OreUnit.h"

class UnitManager {
public:

	void Initalize(MapChipField* mapChipField, DrawData playerDrawData, DrawData oreDrawData, KeyManager* keyManager);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

	/// <summary>
	/// ユニットを追加
	/// </summary>
	/// <param name="targetPos">移動する位置</param>
	void AddOreUnit(const Vector3& targetPos);

private:

	// マップ
	MapChipField* mapChipField_ = nullptr;

	// おれの描画データ
	DrawData oreDrawData_;

	// プレイヤー
	std::unique_ptr<PlayerUnit> playerUnit_;

	// おれ
	// <id, インスタンス>
	std::unordered_map<int32_t, std::unique_ptr<OreUnit>> oreUnits_;
	// 最大のおれの数
	int32_t maxOreCount_ = 100;
	// 現在のid
	int32_t currentId_ = 0;
	// 再利用可能id
	std::deque<int32_t> freeIndices_;

	// おれの出現位置
	std::vector<Vector3> homePosList_;

private:

	// 一番近い出現位置を求める
	Vector3 GetNearHomePos(const Vector3& targetPos);
};

// ヘルプ関数
namespace {

	// XZ上での距離を求める
	float DistanceXZ(const Vector3& v1, const Vector3& v2) {
		float dx = v1.x - v2.x;
		float dz = v1.z - v2.z;
		return dx * dx + dz * dz;
	}
}