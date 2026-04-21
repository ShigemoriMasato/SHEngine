#pragma once
#include <utility>
#include <array>
#include <vector>

struct MovableMino {
	std::vector<std::pair<int, int>> offset;
	std::pair<int, int> position;
	int minoType;
};
