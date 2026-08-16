#pragma once
#include "../Polygon/PolygonEmitter.h"

class RejectBallPolygonEmitter : public PolygonEmitter {
public:

	struct RejectBall {
		Vector3 position = { 0, 0, 0 };
		float radius = 0.0f;
	};

public:

	RejectBallPolygonEmitter(uint32_t maxParticleNum = 1000000) : PolygonEmitter(maxParticleNum) {}

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;

	void SetRejectBalls(const std::array<RejectBall, 16>& rejectBalls);

private:

	SHEngine::GPUBuffer* rejectBalls_ = nullptr;
	SHEngine::GPUBuffer* rejectBallNum_ = nullptr;

	const int kMaxRejectBallNum_ = 16;

};
