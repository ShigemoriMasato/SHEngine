#pragma once
#include <Game/Effect/IgnorePol/IgnoreBallPolygonEmitter.h>

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

	uint32_t SetMove(std::function<IgnoreBallPolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)> moveFunc);
	void RemoveMove(uint32_t index);

	void SetPresetFunc(IgnoreBallPreset preset);

	const std::array<IgnoreBallPolygonEmitter::IgnoreBall, 16>& GetIgnoreBalls() const { return ignoreBalls_; }

	void DrawImGui();

private:

	static constexpr int kMaxIgnoreBallNum_ = 16;
	std::array<IgnoreBallPolygonEmitter::IgnoreBall, kMaxIgnoreBallNum_> ignoreBalls_ = {};
	std::array<std::function<IgnoreBallPolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)>, kMaxIgnoreBallNum_> moveFuncs_ = {};

	std::vector<std::function<IgnoreBallPolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)>> presetFuncs_ = {};
};
