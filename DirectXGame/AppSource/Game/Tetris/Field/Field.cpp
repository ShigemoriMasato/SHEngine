#include "Field.h"
#include <numbers>

namespace nums = std::numbers;

Field::Field(int width, int height) : width_(width), height_(height) {
}

void Field::Initialize(Camera* camera) {
	field_.resize(height_ + 5, std::vector<int>(width_, 0));
}

std::vector<std::vector<int>> Field::GetField() const {
	return field_;
}

bool Field::SetFieldIndex(int x, int y, int type) {
	bool gameover = true;
	if (field_[y][x] != 0) {
		gameover = false;
	}
	field_[y][x] = type;
	return gameover;
}

void Field::LineCheck() {
	for (int i = 0; i < height_; ++i) {
		bool isLineFull = true;
		for (int j = 0; j < width_; ++j) {
			if (field_[i][j] == 0) {
				isLineFull = false;
				break;
			}
		}

		if (isLineFull) {
			reqLine_.push_back(i);
		}
	}
}

void Field::DeleteFillLine() {
	int offset = 0;
	for (const auto& line : reqLine_) {
		field_.erase(field_.begin() + line - offset++);
		field_.emplace_back(std::vector<int>(width_, 0));
	}
	reqLine_.clear();
}
