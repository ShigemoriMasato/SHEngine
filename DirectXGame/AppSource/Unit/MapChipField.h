#pragma once
#include<vector>
#include<array>
#include"Utility/Vector.h"
#include"Game/MapData/Data/MapData.h"

class MapChipField {
public:

	struct IndexSet {
		int32_t xIndex;
		int32_t zIndex;
	};

	// 範囲矩形
	struct Rect {
		float left;   // 左端
		float right;  // 右端
		float bottom; // 下端
		float top;    // 上端
	};

	// 当たり判定
	struct CollisionMapInfo {
		bool isWallHit = false; // 壁との接触フラグ
		Vector3 move = {};      // 移動量

		Vector3 pos = {};       // 位置
		Vector3 size = {};      // サイズ
	};

	// 移動方向
	enum class MoveDir {
		Above, // 上
		Below, // 下
		Right, // 右
		Left,  // 左

		MaxCount  // 要素数
	};

	// 角
	enum class Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		MaxCount, // 要素数
	};

public: // マップの情報

	/// <summary>
	/// マップデータを設定する
	/// </summary>
	/// <param name="data"></param>
	void SetMapChipData(std::vector<std::vector<TileType>> data);

	void SetDebugMapData();

	// マップデータを取得
	std::vector<std::vector<TileType>> GetMapData() { return data_; }

	/// <summary>
	/// 指定されたマップチップデータの種類を返す
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	TileType GetBlockTypeByIndex(int32_t xIndex, int32_t zIndex) const;

	/// <summary>
	/// 指定されたマップチップデータの座標を返す
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	Vector3 GetMapChipPositionByIndex(int32_t xIndex, int32_t zIndex) const;

	/// <summary>
	/// 指定座標がマップチップで何番目かを返す
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	/// <summary>
	/// ブロックの範囲を返す
	/// </summary>
	/// <param name="xIndex"></param>
	/// <param name="yIndex"></param>
	/// <returns></returns>
	Rect GetRectByIndex(int32_t xIndex, int32_t zIndex);

public: // マップの当たり判定

	/// <summary>
	/// ブロックとの当たり判定を受け取る
	/// </summary>
	/// <param name="corner1"></param>
	/// <param name="corner2"></param>
	/// <param name="info"></param>
	/// <returns></returns>
	bool IsBlockHit(MoveDir dir, const CollisionMapInfo& info);

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline int32_t kNumBlockVirtical = 10;
	static inline int32_t kNumBlockHorizontal = 10;

	// マップデータ
	std::vector<std::vector<TileType>> data_;

private:

	/// <summary>
	/// 指定した角の座標を取得
	/// </summary>
	/// <param name="center"></param>
	/// <param name="corner"></param>
	/// <returns></returns>
	Vector3 CornerPosition(const Vector3& center, Corner corner, const CollisionMapInfo& info);
};