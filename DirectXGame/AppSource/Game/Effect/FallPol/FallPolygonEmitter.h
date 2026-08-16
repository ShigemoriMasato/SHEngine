#pragma once
#include "../Polygon/PolygonEmitter.h"

class FallPolygonEmitter : public PolygonEmitter {
public:

	struct Sphere {
		Vector3 pos;
		float radius;
	};

public:

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

	void SetConfig(Config& config) override;

	void Fall(Sphere sphere);

	void SetGravity(Vector3 gravity) {
		gravity_->CopyBuffer(&gravity, sizeof(Vector3));
	}
	void SetLifeTime(float lifeTime) {
		lifeTime_->CopyBuffer(&lifeTime, sizeof(float));
	}

private:

	std::unique_ptr<SHEngine::ComputeObject> fallSphere_ = nullptr;

	SHEngine::GPUBuffer* isFall_ = nullptr;
	SHEngine::GPUBuffer* gravity_ = nullptr;

	SHEngine::GPUBuffer* spheres_ = nullptr;
	SHEngine::GPUBuffer* sphereNum_ = nullptr;

	const uint32_t kMaxSphereNum_ = 64;


	std::vector<Sphere> sphereList_ = {};
};
