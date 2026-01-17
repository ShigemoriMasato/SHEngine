#include"MapChipField.h"

void MapChipField::SetMapChipData(std::vector<std::vector<TileType>> data) {
	// データを取得
	data_ = data;

	// ブロックの縦と横幅を取得
	kNumBlockVirtical = static_cast<int32_t>(data_.size());
	kNumBlockHorizontal = static_cast<int32_t>(data_[0].size());
}

void MapChipField::SetDebugMapData() {

	kNumBlockVirtical = 40;
	kNumBlockHorizontal = 60;

	data_.resize(kNumBlockVirtical);
	for (auto& row : data_) {
		row.resize(kNumBlockHorizontal);
	}

	// デバックのマップデータを取得
	for (int32_t z = 0; z < kNumBlockVirtical; ++z) {
		for (int32_t x = 0; x < kNumBlockHorizontal; ++x) {
			
			// 外周をブロックで囲む
			if (z == 0 || z == kNumBlockVirtical-1 || x == 0 || x == kNumBlockHorizontal-1) {
				data_[z][x] = TileType::Wall;
			} else {
				data_[z][x] = TileType::Air;
			}
		}
	}

	data_[5][4] = TileType::Wall;
	data_[6][4] = TileType::Wall;
	data_[5][5] = TileType::Wall;
	data_[6][5] = TileType::Wall;
}

TileType MapChipField::GetBlockTypeByIndex(int32_t xIndex, int32_t zIndex) const {

	// 範囲外を指定されたら空白を返す
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return TileType::Air;
	}
	if (zIndex < 0 || kNumBlockVirtical - 1 < zIndex) {
		return TileType::Air;
	}

	return data_[zIndex][xIndex];
}

Vector3 MapChipField::GetMapChipPositionByIndex(int32_t xIndex, int32_t zIndex) const {
	return Vector3(kBlockWidth * static_cast<float>(xIndex), 0, static_cast<float>(kBlockHeight * zIndex));
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	// 指定座標がマップチップで何番目かを返す
	MapChipField::IndexSet indexSet = {};
	indexSet.xIndex = static_cast<int32_t>((position.x + kBlockWidth / 2.0f) / kBlockWidth);
	indexSet.zIndex = static_cast<int32_t>((position.z + kBlockHeight / 2.0f) / kBlockHeight);
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(int32_t xIndex, int32_t zIndex) {

	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, zIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	return rect;
}

bool MapChipField::IsBlockHit(MoveDir dir, const CollisionMapInfo& info) {

	// 自キャラの移動後の4つ角の座標を求める
	std::array<Vector3, static_cast<size_t>(Corner::MaxCount)> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(info.pos + info.move, static_cast<Corner>(i),info);
	}

	// 角
	uint32_t corner1 = 0;
	uint32_t corner2 = 0;

	// 移動先
	int32_t moveX = 0;
	int32_t moveZ = 0;

	// 方向に応じて角を求める
	switch (dir)
	{
	case MapChipField::MoveDir::Above:
		corner1 = static_cast<uint32_t>(Corner::kLeftTop);
		corner2 = static_cast<uint32_t>(Corner::kRightTop);
		moveZ = -1;
		break;

	case MapChipField::MoveDir::Below:
		corner1 = static_cast<uint32_t>(Corner::kLeftBottom);
		corner2 = static_cast<uint32_t>(Corner::kRightBottom);
		moveZ = 1;
		break;

	case MapChipField::MoveDir::Right:
		corner1 = static_cast<uint32_t>(Corner::kRightTop);
		corner2 = static_cast<uint32_t>(Corner::kRightBottom);
		moveX = -1;
		break;

	case MapChipField::MoveDir::Left:
		corner1 = static_cast<uint32_t>(Corner::kLeftTop);
		corner2 = static_cast<uint32_t>(Corner::kLeftBottom);
		moveX = 1;
		break;
	}

	TileType blockType;
	TileType blockTypeNext;
	// 角1、角2の当たり判定を行う
	bool hit = false;
	// 角1の点の判定
	MapChipField::IndexSet indexSet;
	indexSet = GetMapChipIndexSetByPosition(positionsNew[corner1]);
	blockType = GetBlockTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	blockTypeNext = GetBlockTypeByIndex(indexSet.xIndex + moveX, indexSet.zIndex + moveZ);
	if ((blockType != TileType::Air) && (blockTypeNext == TileType::Air)) {
		hit = true;
	}
	// 角2の点の判定
	indexSet = GetMapChipIndexSetByPosition(positionsNew[corner2]);
	blockType = GetBlockTypeByIndex(indexSet.xIndex, indexSet.zIndex);
	blockTypeNext = GetBlockTypeByIndex(indexSet.xIndex + moveX, indexSet.zIndex + moveZ);
	if ((blockType != TileType::Air) && (blockTypeNext == TileType::Air)) {
		hit = true;
	}

	return hit;
}

Vector3 MapChipField::CornerPosition(const Vector3& center, Corner corner, const CollisionMapInfo& info) {

	// 各角の座標
	Vector3 offsetTable[static_cast<size_t>(Corner::MaxCount)] = {
		Vector3(info.size.x / 2.0f,0.0f, -info.size.z / 2.0f),
		Vector3(-info.size.x / 2.0f,0.0f, -info.size.z / 2.0f),
		Vector3(info.size.x / 2.0f,0.0f, info.size.z / 2.0f),
		Vector3(-info.size.x / 2.0f,0.0f, info.size.z / 2.0f)
	};

	return center + offsetTable[static_cast<int32_t>(corner)];
}