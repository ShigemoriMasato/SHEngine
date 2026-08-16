#pragma once
#include <Game/Effect/RejectPol/RejectBallPolygonEmitter.h>

enum class RejectBallPreset {
	None,
	Round,
	Scale,
	Impact,
	Count,
};

class RejectBallManager {
public:

	void Initialize();
	void Update(float deltaTime);

	uint32_t SetMove(std::function<RejectBallPolygonEmitter::RejectBall(float deltaTime, bool& destroyMe)> moveFunc);
	void RemoveMove(uint32_t index);

	void SetPresetFunc(RejectBallPreset preset);

	const std::array<RejectBallPolygonEmitter::RejectBall, 16>& GetRejectBalls() const { return rejectBalls_; }

	void DrawImGui();

private:

	static constexpr int kMaxRejectBallNum_ = 16;
	std::array<RejectBallPolygonEmitter::RejectBall, kMaxRejectBallNum_> rejectBalls_ = {};
	std::array<std::function<RejectBallPolygonEmitter::RejectBall(float deltaTime, bool& destroyMe)>, kMaxRejectBallNum_> moveFuncs_ = {};

	std::vector<std::function<RejectBallPolygonEmitter::RejectBall(float deltaTime, bool& destroyMe)>> presetFuncs_ = {};
};
