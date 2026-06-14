#pragma once
#include "Pool/ParticlePool.h"
#include "Wave/WaveParticle.h"
#include <SHEngine.h>
#include "Subject/Subject.h"

class Effect {
public:

	void Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine);

	void Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime);

	void Draw(SHEngine::Screen::IDisplay* display);

private:

	SHEngine::Engine* engine_ = nullptr;
	SHEngine::TextureManager* textureManager_ = nullptr;

	SHEngine::ComputeCommandContext* compute_;
	SHEngine::DirectCommandContext* direct_;
	std::unique_ptr<ParticlePool> particlePool_ = nullptr;

	std::unique_ptr<WaveParticle> waveParticle_ = nullptr;

};
