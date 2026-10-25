#pragma once
#include <Particle/System/ParticleProcessor.h>
#include <Particle/System/ParticleRenderer.h>
#include <Particle/Emitter.h>

class ParticleSystem {
public:

	ParticleSystem(uint32_t particleCount);

	void Initialize(ICC* icc);
	void Update(ICC* icc);
	void Draw(DCC* dcc);

	Particle::Emitter* CreateEmitter(int countNum, ICC* icc);

	void SetCamera(const Camera* camera);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	Particle::Data data_;
	Particle::FreeList freeList_;

	std::unique_ptr<Particle::Processor> processor_;
	std::unique_ptr<Particle::Renderer> renderer_;

	std::vector<std::unique_ptr<Particle::Emitter>> emitters_;

	uint32_t nextEmitterID_ = 0;
};
