#pragma once
#include "../Polygon/PolygonEmitter.h"

class IgnoreBallPolygonEmitter : public PolygonEmitter {
public:

	struct IgnoreBall {
		Vector3 position = { 0, 0, 0 };
		float radius = 0.0f;
	};

public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;

	void SetIgnoreBalls(const std::array<IgnoreBall, 16>& ignoreBalls);

private:

	SHEngine::GPUBuffer* ignoreBalls_ = nullptr;
	SHEngine::GPUBuffer* ignoreBallNum_ = nullptr;

	const int kMaxIgnoreBallNum_ = 16;

};
