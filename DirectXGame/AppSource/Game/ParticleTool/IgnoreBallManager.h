#pragma once
#include <Game/Effect/Polygon/PolygonEmitter.h>

enum class IgnoreBallPreset {
	None,
	Round,
	Scale,
	Impact,
	Count,
};

class IgnoreBallManager {
public:

	void Initialize();
	void Update(float deltaTime);

	uint32_t SetMove(std::function<PolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)> moveFunc);
	void RemoveMove(uint32_t index);

	void SetPresetFunc(IgnoreBallPreset preset);

	const std::array<PolygonEmitter::IgnoreBall, 16>& GetIgnoreBalls() const { return ignoreBalls_; }

	void DrawImGui();

private:

	static constexpr int kMaxIgnoreBallNum_ = 16;
	std::array<PolygonEmitter::IgnoreBall, kMaxIgnoreBallNum_> ignoreBalls_ = {};
	std::array<std::function<PolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)>, kMaxIgnoreBallNum_> moveFuncs_ = {};

	std::vector<std::function<PolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)>> presetFuncs_ = {};
};
