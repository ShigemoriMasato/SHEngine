#include "IgnoreBallPolygonEmitter.h"

void IgnoreBallPolygonEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	PolygonEmitter::Initialize(engine, pool);

	ignoreBalls_ = container_->Create(BufferType::SRV, sizeof(IgnoreBall), 16);
	ignoreBallNum_ = container_->Create(BufferType::CBV, sizeof(int));

	ignoreBallNum_->CopyBuffer(&kMaxIgnoreBallNum_, sizeof(int));

	SetUpdate({}, { ignoreBalls_ }, { ignoreBallNum_ }, "IgnoreBall.CS.hlsl");
}

void IgnoreBallPolygonEmitter::SetIgnoreBalls(const std::array<IgnoreBall, 16>& ignoreBalls) {
	ignoreBalls_->CopyBuffer(ignoreBalls.data(), sizeof(IgnoreBall) * ignoreBalls.size());
}
