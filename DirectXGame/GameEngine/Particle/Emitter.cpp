#include "Emitter.h"

Particle::Emitter::Emitter(uint32_t emitterID, uint32_t particleCount, Data* data, FreeList* freeList, Processor* processor, ICC* icc, SHEngine::BufferContainer* container) : 
	processorKey_(processor->GetKey()),
	countNum_(particleCount),
	id_(emitterID),
	processor_(processor) {
	data_ = data;
	indexList_ = container->Create(BufferType::SRV_UAV, sizeof(uint32_t), countNum_, BufferNum::Single);
	emitterID_ = container->Create(BufferType::CBV, sizeof(uint32_t));
	emitterID_->CopyBuffer(&emitterID, sizeof(emitterID));
	countBuffer_ = container->Create(BufferType::CBV, sizeof(uint32_t));
	countBuffer_->CopyBuffer(&countNum_, sizeof(countNum_));

	processor->Allocate(indexList_, emitterID_, countBuffer_, countNum_, icc);
}

uint32_t Particle::Emitter::AddProcess(SHEngine::ComputeObject*& output) {
	int id = nextProcessID_++;
	auto process = std::make_unique<SHEngine::ComputeObject>("ParticleProcess: " + std::to_string(id));

	process->SetGPUBuffer(BufferType::CBV, countBuffer_);
	process->SetGPUBuffer(BufferType::SRV, indexList_);
	process->SetGPUBuffers(BufferType::UAV, { data_->positions, data_->scales, data_->colors });

	output = process.get();
	processes_[id] = std::move(process);
	return id;
}

void Particle::Emitter::AddTriggerProcess(SHEngine::ComputeObject*& output) {
	auto process = std::make_unique<SHEngine::ComputeObject>("ParticleProcess: Trigger");

	process->SetGPUBuffer(BufferType::CBV, countBuffer_);
	process->SetGPUBuffer(BufferType::SRV, indexList_);
	process->SetGPUBuffers(BufferType::UAV, { data_->positions, data_->scales, data_->colors });

	output = process.get();
	triggerProcesses_.push_back(std::move(process));
}

void Particle::Emitter::EraseProcess(uint32_t id) {
	const auto& it = processes_.find(id);
	if (it != processes_.end()) {
		processes_.erase(it);
	}
}

void Particle::Emitter::Update(uint32_t processorKey, ICC* icc) {
	if (processorKey != processorKey_) {
		return;
	}

	for (auto& triggerProcess : triggerProcesses_) {
		triggerProcess->Execute(icc);
	}

	triggerProcesses_.clear();

	for (auto& [id, process] : processes_) {
		process->Execute(icc);
	}
}

void Particle::Emitter::SetParentMatrix(const Matrix4x4& matrix) {
	processor_->SetParentMatrix(id_, matrix);
}
