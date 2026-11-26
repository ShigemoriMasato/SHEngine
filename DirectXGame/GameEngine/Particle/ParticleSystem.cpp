#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(uint32_t particleCount, bool isCube) {
	container_ = std::make_unique<SHEngine::BufferContainer>(32, "ParticleSystem");

	data_.Initialize(container_.get(), particleCount);
	freeList_.Initialize(container_.get(), particleCount);

	processor_ = std::make_unique<Particle::Processor>(&data_, &freeList_);

	renderer_ = std::make_unique<Particle::Renderer>();
	renderer_->Initialize(&data_, container_.get(), isCube);
	renderer_->SetInstance(particleCount);
}

void ParticleSystem::Initialize(ICC* icc) {
	processor_->Initialize(icc);
}

void ParticleSystem::Update(ICC* icc) {
	renderer_->Update();
	processor_->Update();

	for (auto& emitter : emitters_) {
		emitter->Update(processor_->GetKey(), icc);
	}
}

void ParticleSystem::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

Particle::Emitter* ParticleSystem::CreateEmitter(int countNum, ICC* icc, const std::string& name) {
	const uint32_t id = nextEmitterID_++;
	const std::string emitterName = name.empty() ? "Emitter" + std::to_string(id) : name;
	auto emitter = std::make_unique<Particle::Emitter>(id, countNum, &data_, &freeList_, processor_.get(), icc, container_.get(), emitterName);
	emitters_.push_back(std::move(emitter));
	return emitters_.back().get();
}

void ParticleSystem::EraseEmitter(Particle::Emitter* emitter) {
	for (auto it = emitters_.begin(); it != emitters_.end(); ++it) {
		if (it->get() == emitter) {
			emitter->Release();
			emitters_.erase(it);
			break;
		}
	}
}

void ParticleSystem::SetCamera(const Camera* camera) {
	renderer_->SetCamera(camera);
}
