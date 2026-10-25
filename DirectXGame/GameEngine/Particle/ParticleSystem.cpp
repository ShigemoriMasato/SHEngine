#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(uint32_t particleCount) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	data_.Initialize(container_.get(), particleCount);
	freeList_.Initialize(container_.get(), particleCount);

	processor_ = std::make_unique<Particle::Processor>(&data_, &freeList_);

	renderer_ = std::make_unique<Particle::Renderer>();
	renderer_->Initialize(&data_, container_.get());
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

Particle::Emitter* ParticleSystem::CreateEmitter(int countNum, ICC* icc) {
	auto emitter = std::make_unique<Particle::Emitter>(nextEmitterID_++, countNum, &data_, &freeList_, processor_.get(), icc, container_.get());
	emitters_.push_back(std::move(emitter));
	return emitters_.back().get();
}

void ParticleSystem::SetCamera(const Camera* camera) {
	renderer_->SetCamera(camera);
}
