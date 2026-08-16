#include "RejectBallPolygonEmitter.h"

void RejectBallPolygonEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	PolygonEmitter::Initialize(engine, pool);

	rejectBalls_ = container_->Create(BufferType::SRV, sizeof(RejectBall), 16);
	rejectBallNum_ = container_->Create(BufferType::CBV, sizeof(int));

	rejectBallNum_->CopyBuffer(&kMaxRejectBallNum_, sizeof(int));

	SetUpdate({}, { rejectBalls_ }, { rejectBallNum_ }, "RejectBall.CS.hlsl");
}

void RejectBallPolygonEmitter::SetRejectBalls(const std::array<RejectBall, 16>& rejectBalls) {
	rejectBalls_->CopyBuffer(rejectBalls.data(), sizeof(RejectBall) * rejectBalls.size());
}
