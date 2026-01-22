#include "MapData.h"
#include <random>

std::vector<std::vector<TileType>> MapDataForBin::GetDirectionGrid(const Direction& direction, float goldFrequency) const {
	std::vector<std::vector<TileType>> ans;
	std::vector<std::pair<int, int>> goldPositions;
	int index = 0;
	switch (direction) {
	case Direction::Front:
		ans.resize(height, std::vector<TileType>(width, TileType::Air));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				index = y * width + x;
				ans[y][x] = tileData[index];
				if (tileData[index] == TileType::Gold) {
					goldPositions.emplace_back(x, y);
				}
			}
		}
		break;

	case Direction::Right:
		ans.resize(width, std::vector<TileType>(height, TileType::Air));
		for (int y = 0; y < width; ++y) {
			for (int x = 0; x < height; ++x) {
				index = width * (height - 1 - x) + y;
				ans[y][x] = tileData[index];
				if (tileData[index] == TileType::Gold) {
					goldPositions.emplace_back(x, y);
				}
			}
		}
		break;

	case Direction::Back:
		ans.resize(height, std::vector<TileType>(width, TileType::Air));
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				index = (height - 1 - y) * width + (width - 1 - x);
				ans[y][x] = tileData[index];
				if (tileData[index] == TileType::Gold) {
					goldPositions.emplace_back(x, y);
				}
			}
		}
		break;

	case Direction::Left:
		ans.resize(width, std::vector<TileType>(height, TileType::Air));
		for (int y = 0; y < width; ++y) {
			for (int x = 0; x < height; ++x) {
				index = (width - 1 - y) + width * x;
				ans[y][x] = tileData[index];
				if (tileData[index] == TileType::Gold) {
					goldPositions.emplace_back(x, y);
				}
			}
		}
		break;
	}

	//金の抽選
	int goldNum = static_cast<int>(goldPositions.size() * goldFrequency);
	std::shuffle(goldPositions.begin(), goldPositions.end(), std::mt19937(std::random_device()()));

	for (int i = 0; i < goldNum; ++i) {
		auto [x, y] = goldPositions[i];
		ans[y][x] = TileType::Gold;
	}

	return ans;
}

bool MapDataForBin::Verify() const {
	bool valid = true;
	if (width <= 0 || height <= 0) {
		valid = false;
	}
	if (modelFilePath.empty()) {
		valid = false;
	}
	if (tileData.size() != static_cast<size_t>(width * height)) {
		assert(false && "MapDataForBin::Verify() failed: mapSize is Defferent");
		valid = false;
	}
	if (mapID < 0) {
		assert(false && "MapDataForBin::Verify() failed: mapID is negative");
		valid = false;
	}
	return valid;
}

bool MapData::Verify() const {
	bool valid = true;
	if (width <= 0 || height <= 0) {
		valid = false;
	}
	if (modelFilePath.empty()) {
		valid = false;
	}
	if (tileGrid.size() != static_cast<size_t>(height)) {
		assert(false && "MapData::Verify() failed: tileGrid height mismatch");
		valid = false;
	} else {
		for (const auto& row : tileGrid) {
			if (row.size() != static_cast<size_t>(width)) {
				assert(false && "MapData::Verify() failed: tileGrid width mismatch");
				valid = false;
				break;
			}
		}
	}
	if (mapID < 0) {
		assert(false && "MapData::Verify() failed: mapID is negative");
		valid = false;
	}
	return valid;
}
