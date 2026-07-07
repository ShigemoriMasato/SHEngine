#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Compute/ComputeObject.h>
#include "../Pool/ParticlePool.h"

class IEmitter {
public:

	IEmitter(uint32_t maxParticleNum) : maxParticleNum_(maxParticleNum) {}

	virtual void Initialize(SHEngine::Engine* engine, const Pool& pool, const uint32_t offset) = 0;

	virtual void Update(CmdObj* compute, float deltaTime) = 0;
	virtual void Draw(DCC* direct) = 0;

	uint32_t GetMaxParticleNum() const { return maxParticleNum_; }

protected:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	const uint32_t maxParticleNum_ = 0;

};
