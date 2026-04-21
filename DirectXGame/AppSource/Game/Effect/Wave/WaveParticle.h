#pragma once
#include <Compute/ComputeObject.h>
#include <Render/Renderer.h>
#include <Game/Effect/Pool/ParticlePool.h>

class WaveParticle {
public:

	void Initialize(const Pool& pool, const uint32_t id);
	void Update(CmdObj* compute);

	void DrawImGui();

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	std::unique_ptr<SHEngine::ComputeObject> emitter_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	SHEngine::GPUBuffer* lifeTime_ = nullptr;
	SHEngine::GPUBuffer* speed_ = nullptr;
	SHEngine::GPUBuffer* seed_ = nullptr;
	SHEngine::GPUBuffer* emitNum_ = nullptr;

	struct Config {
		float speed = 1.0f;
		float seed = 0.0f;
		float lifeTime = 1.0f;
		int emitNum = 3;
	}config_;
};
