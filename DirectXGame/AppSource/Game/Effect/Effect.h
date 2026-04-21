#pragma once
#include "Pool/ParticlePool.h"
#include "Wave/WaveParticle.h"
#include <SHEngine.h>

class Effect {
public:

	void Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine);

	void Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix);

	void Draw(SHEngine::Screen::IDisplay* display);

private:

	SHEngine::Engine* engine_ = nullptr;

	std::vector<std::unique_ptr<CmdObj>> compute_ = {};
	std::unique_ptr<CmdObj> update_ = nullptr;
	std::unique_ptr<CmdObj> direct_ = nullptr;
	std::vector<SHEngine::Command::WaitFence> computeFence_{};
	SHEngine::Command::WaitFence drawFence_{};

	std::unique_ptr<ParticlePool> particlePool_ = nullptr;

	std::unique_ptr<WaveParticle> waveParticle_ = nullptr;
};
