#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Compute/ComputeObject.h>
#include "../Pool/ParticlePool.h"

class IEmitter {
public:

	IEmitter() = default;

	virtual void Initialize(SHEngine::Engine* engine, const Pool& pool) = 0;
	virtual void Update(CCC* compute, float deltaTime) = 0;
	uint32_t GetMaxParticleNum() const { return maxParticleNum_; }

protected:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	const uint32_t maxParticleNum_ = 0;

};
