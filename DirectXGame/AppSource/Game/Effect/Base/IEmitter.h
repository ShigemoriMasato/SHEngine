#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <Compute/ComputeObject.h>
#include <random>
#include "../Pool/ParticlePool.h"

class IEmitter {
public:

	IEmitter() = default;

	virtual void Initialize(SHEngine::Engine* engine, const Pool& pool) = 0;
	virtual void Update(CCC* compute, float deltaTime) = 0;

protected:

	uint32_t GetRandU();
	float GetRandF();

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

private:

	static inline std::mt19937 randomEngine_{ std::random_device{}() };
	static inline std::uniform_int_distribution<uint32_t> uintDist_{ 0, UINT32_MAX };
	static inline std::uniform_real_distribution<float> floatDist_{ 0.0f, 1.0f };

};
