#pragma once
#include <Utility/DataStructures.h>
#include <vector>

struct SprashConfig {
    float spread;
    Vector3 emitPos;
    float speed;

    Vector4 color;
    float scale;
};

struct RollingSprashData {
	std::vector<SprashConfig> sprashConfigs;
};
