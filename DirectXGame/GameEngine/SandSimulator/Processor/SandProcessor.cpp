#include "SandProcessor.h"

Sand::Processor::Processor() {
	auto CreateCO = [](std::string name) {
		auto co = std::make_unique<SHEngine::ComputeObject>("SandSimulator::" + name);
		co->Initialize();
		co->SetShader("Sand/System/" + name + ".CS.hlsl");
		return co;
		};

	initialize_ = CreateCO("Initialize");
	gridInitialize_ = CreateCO("GridInitialize");
	sendDensity_ = CreateCO("SendDensity");
	addForce_ = CreateCO("AddForce");
	particleMove_ = CreateCO("ParticleMove");
}

void Sand::Processor::Initialize(Sand::Particle* particle, Sand::Grid* grid, Sand::FreeList* freeList, SHEngine::ICommandContext* icc) {
	initialize_->SetGPUBuffer(BufferType::CBV, freeList->count);
	initialize_->SetGPUBuffers(BufferType::SRV, { freeList->list, freeList->index, particle->isAlive });
	initialize_->SetExecuteNum(freeList->countNum);
	initialize_->Execute(icc);

	gridInitialize_->SetGPUBuffer(BufferType::CBV, grid->gridCount);
	gridInitialize_->SetGPUBuffer(BufferType::UAV, grid->density);
	gridInitialize_->SetExecuteNum(grid->gridNum);

	sendDensity_->SetGPUBuffers(BufferType::CBV, { particle->count, grid->gridCount, grid->gridSize, grid->gridStartPos });
	sendDensity_->SetGPUBuffers(BufferType::SRV, { particle->position, particle->isAlive });
	sendDensity_->SetGPUBuffer(BufferType::UAV, grid->density);
	sendDensity_->SetExecuteNum(particle->countNum);

	addForce_->SetGPUBuffers(BufferType::CBV, { particle->count, grid->gridCount, grid->gridSize, grid->gridStartPos, grid->densityMax });
	addForce_->SetGPUBuffers(BufferType::SRV, { particle->position, particle->isAlive, grid->density });
	addForce_->SetGPUBuffer(BufferType::UAV, particle->force);
	addForce_->SetExecuteNum(particle->countNum, 128);

	particleMove_->SetGPUBuffers(BufferType::CBV, { particle->count, particle->scale });
	particleMove_->SetGPUBuffer(BufferType::SRV, particle->isAlive);
	particleMove_->SetGPUBuffers(BufferType::UAV, { particle->position, particle->velocity, particle->force });
}
