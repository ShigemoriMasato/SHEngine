#pragma once
#include "Pool/ParticlePool.h"
#include "Wave/WaveParticle.h"
#include <SHEngine.h>
#include "Subject/Subject.h"

class Effect {
public:

	void Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine, SHEngine::TextureManager* textureManager);

	void Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime);

	void Draw(SHEngine::Screen::IDisplay* display);

private:

	SHEngine::Engine* engine_ = nullptr;
	SHEngine::TextureManager* textureManager_ = nullptr;

	std::unique_ptr<SHEngine::Command::Object> compute_ = nullptr;
	std::unique_ptr<SHEngine::Command::Object> direct_ = nullptr;
	SHEngine::Command::WaitFence computeFence_{};

	std::unique_ptr<ParticlePool> particlePool_ = nullptr;

	std::unique_ptr<WaveParticle> waveParticle_ = nullptr;

};
