#include "Emitter.h"

Particle::Emitter::Emitter(uint32_t emitterID, uint32_t particleCount, Data* data, FreeList* freeList, Processor* processor, ICC* icc, SHEngine::BufferContainer* container, const std::string& name) :
	processorKey_(processor->GetKey()),
	countNum_(particleCount),
	id_(emitterID),
	processor_(processor),
	name_(name) {
	data_ = data;
	// Resourceウィンドウでは "ParticleSystem/<エミッター名>" に出る
	SHEngine::BufferContainer* own = container->CreateChild(name_);
	indexList_ = own->Create(BufferType::SRV_UAV, sizeof(uint32_t), countNum_, BufferNum::Single, "IndexList");
	emitterID_ = own->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::MatchSwapChain, "EmitterID");
	emitterID_->CopyBuffer(&emitterID, sizeof(emitterID));
	countBuffer_ = own->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::MatchSwapChain, "ParticleCount");
	countBuffer_->CopyBuffer(&countNum_, sizeof(countNum_));

	processor->Allocate(indexList_, emitterID_, countBuffer_, countNum_, icc);
}

SHEngine::ComputeObject* Particle::Emitter::AddProcess(std::string shaderPath, uint32_t* output) {
	int id = nextProcessID_++;
	auto process = std::make_unique<SHEngine::ComputeObject>("ParticleProcess: " + std::to_string(id));

	process->SetGPUBuffer(BufferType::CBV, 0, countBuffer_);
	process->SetGPUBuffer(BufferType::SRV, 0, indexList_);
	process->SetGPUBuffers(BufferType::UAV, 0, { data_->positions, data_->rotations, data_->scales, data_->colors });

	process->SetShader("Particle/" + shaderPath + ".CS.hlsl");

	if (output) {
		*output = id;
	}

	processes_[id] = std::move(process);
	return processes_[id].get();
}

SHEngine::ComputeObject* Particle::Emitter::AddTriggerProcess(std::string shaderPath) {
	auto process = std::make_unique<SHEngine::ComputeObject>("ParticleProcess: Trigger");

	process->SetGPUBuffer(BufferType::CBV, 0, countBuffer_);
	process->SetGPUBuffer(BufferType::SRV, 0, indexList_);
	process->SetGPUBuffers(BufferType::UAV, 0, { data_->positions, data_->rotations, data_->scales, data_->colors });

	process->SetShader("Particle/" + shaderPath + ".CS.hlsl");

	auto output = process.get();
	triggerProcesses_.push_back(std::move(process));
	return output;
}



std::unique_ptr<SHEngine::ComputeObject> Particle::Emitter::AddOprionalProcess(std::string shaderPath) {
	auto process = std::make_unique<SHEngine::ComputeObject>("ParticleProcess: Optional");

	process->SetGPUBuffer(BufferType::CBV, 0, countBuffer_);
	process->SetGPUBuffer(BufferType::SRV, 0, indexList_);
	process->SetGPUBuffers(BufferType::UAV, 0, { data_->positions, data_->rotations, data_->scales, data_->colors });

	process->SetShader("Particle/" + shaderPath + ".CS.hlsl");

	return process;
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

	if (isReleased_) {
		return;
	}

	for (auto& triggerProcess : triggerProcesses_) {
		triggerProcess->Execute(icc);
	}

	triggerProcesses_.clear();

	// Requestされたグラフ(1回だけ)
	for (SHEngine::ComputeTrigger* trigger : requests_) {
		trigger->Fire(icc);
	}
	requests_.clear();

	for (auto& [id, process] : processes_) {
		process->Execute(icc);
	}

	if (updateTrigger_) {
		updateTrigger_->Fire(icc);
	}
}

SHEngine::ComputeTrigger* Particle::Emitter::CreateTrigger(const std::string& triggerName) {
	auto it = triggers_.find(triggerName);
	if (it != triggers_.end()) {
		return it->second.get();
	}

	auto trigger = std::make_unique<SHEngine::ComputeTrigger>("Particle/" + name_ + "/" + triggerName);

	// AddProcessと同じ割り当てを、何もつながっていないピンの既定にする
	trigger->BindDefault(BufferType::CBV, 0, countBuffer_, "ParticleCount");
	trigger->BindDefault(BufferType::SRV, 0, indexList_, "IndexList");
	trigger->BindDefault(BufferType::UAV, 0, data_->positions, "Positions");
	trigger->BindDefault(BufferType::UAV, 1, data_->rotations, "Rotations");
	trigger->BindDefault(BufferType::UAV, 2, data_->scales, "Scales");
	trigger->BindDefault(BufferType::UAV, 3, data_->colors, "Colors");

	// 別のレジスタで使いたい場合用に、引数としても渡しておく
	trigger->Bind("ParticleCount", countBuffer_);
	trigger->Bind("IndexList", indexList_);
	trigger->Bind("Positions", data_->positions);
	trigger->Bind("Rotations", data_->rotations);
	trigger->Bind("Scales", data_->scales);
	trigger->Bind("Colors", data_->colors);
	trigger->SetCount("Particles", countNum_);

	SHEngine::ComputeTrigger* result = trigger.get();
	triggers_[triggerName] = std::move(trigger);
	return result;
}

SHEngine::ComputeTrigger* Particle::Emitter::GetTrigger(const std::string& triggerName) const {
	auto it = triggers_.find(triggerName);
	return it != triggers_.end() ? it->second.get() : nullptr;
}

SHEngine::ComputeTrigger* Particle::Emitter::SetUpdateGraph(const std::string& triggerName) {
	updateTrigger_ = CreateTrigger(triggerName);
	return updateTrigger_;
}

void Particle::Emitter::Request(const std::string& triggerName) {
	SHEngine::ComputeTrigger* trigger = GetTrigger(triggerName);
	if (!trigger) {
		static Logger logger = GetLogger("Engine");
		logger->error("Particle::Emitter({})::Request: \"{}\" はCreateTriggerされていません", name_, triggerName);
		return;
	}
	requests_.push_back(trigger);
}

void Particle::Emitter::SetParentMatrix(const Matrix4x4& matrix) {
	processor_->SetParentMatrix(id_, matrix);
}

std::vector<SHEngine::GPUBuffer*> Particle::Emitter::GetBuffer() const {
	return {
		countBuffer_,
		indexList_,
		data_->positions, data_->scales, data_->colors
	};
}
