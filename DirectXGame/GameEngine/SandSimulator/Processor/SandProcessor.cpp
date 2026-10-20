#include "SandProcessor.h"
#include <imgui/imgui.h>

Sand::Processor::Processor(SHEngine::BufferContainer* container, Particle* particle, Grid* grid, Configure* configure) : maxProcessCount_(particle->countNum) {
	auto createCO = [&](const std::string& name) {
		auto co = std::make_unique<SHEngine::ComputeObject>("SandSimulator::" + name);
		co->SetShader("Sand/System/" + name + ".CS.hlsl");
		return co;
		};

	//　定数確認用
	const GridConfig gridConfig;

	ballShape_ = createCO("BallShape");

	deltaTime_ = container->Create(BufferType::CBV, sizeof(float));
	seed_ = container->Create(BufferType::CBV, sizeof(uint32_t));
	ballShapeData_ = container->Create(BufferType::CBV, sizeof(float) * 4);
	gravity_ = container->Create(BufferType::CBV, sizeof(Vector3));
	forceData_ = container->Create(BufferType::CBV, sizeof(Vector3));
	cellCountPerBlockBuffer_ = container->Create(BufferType::CBV, sizeof(uint32_t));
	commonUpdateData_ = { 0.0f, 0.75f, 0.0f };
	forceData_->CopyBuffer(&commonUpdateData_, sizeof(Vector3));
	gravity_->CopyBuffer(&gravityData_, sizeof(Vector3));

	ballShape_->SetExecuteNum(particle->countNum, 128);
	ballShape_->SetGPUBuffers(BufferType::CBV, { particle->count, seed_, ballShapeData_ });
	ballShape_->SetGPUBuffers(BufferType::UAV, { particle->position, particle->force, particle->velocity });

	acceptForce_ = createCO("AcceptForce");
	acceptForce_->SetExecuteNum(particle->countNum, 128);
	acceptForce_->SetGPUBuffers(BufferType::CBV, { particle->count, deltaTime_, forceData_, gravity_ });
	acceptForce_->SetGPUBuffers(BufferType::UAV, { particle->position, particle->force, particle->velocity });

	frameInitialize_ = createCO("FrameInitialize");
	frameInitialize_->SetExecuteNum(gridConfig.cellCount, 128);
	frameInitialize_->SetGPUBuffers(BufferType::CBV, { grid->gridConfig });
	frameInitialize_->SetGPUBuffers(BufferType::UAV, { grid->particleCountInCell, grid->indexWriteBuffer });

	counter_ = createCO("Counter");
	counter_->SetExecuteNum(particle->countNum / 8 + 1, 128);
	counter_->SetGPUBuffers(BufferType::CBV, { particle->count, grid->gridConfig });
	counter_->SetGPUBuffer(BufferType::SRV, particle->position);
	counter_->SetGPUBuffer(BufferType::UAV, grid->particleCountInCell);

	splitBlock_ = createCO("SplitBlock");
	splitBlock_->SetExecuteNum(gridConfig.cellCount, 128);
	splitBlock_->SetGPUBuffers(BufferType::CBV, { grid->gridConfig, cellCountPerBlockBuffer_ });
	splitBlock_->SetGPUBuffers(BufferType::SRV, { grid->particleCountInCell });
	splitBlock_->SetGPUBuffers(BufferType::UAV, { grid->blockNumber });

	prefixSum_ = createCO("PrefixSum");
	prefixSum_->SetExecuteNum(gridConfig.cellCount, 128);
	prefixSum_->SetGPUBuffers(BufferType::CBV, { grid->gridConfig, cellCountPerBlockBuffer_ });
	prefixSum_->SetGPUBuffers(BufferType::SRV, { grid->blockNumber, grid->particleCountInCell });
	prefixSum_->SetGPUBuffers(BufferType::UAV, { grid->beginIndexForCell });

	assignCell_ = createCO("AssignCell");
	assignCell_->SetExecuteNum(particle->countNum / 8 + 1, 128);
	assignCell_->SetGPUBuffers(BufferType::CBV, { particle->count, grid->gridConfig });
	assignCell_->SetGPUBuffers(BufferType::SRV, { particle->position, grid->beginIndexForCell });
	assignCell_->SetGPUBuffers(BufferType::UAV, { grid->particleIndicesForCell, grid->indexWriteBuffer });

	collider_ = createCO("Collider");
	collider_->SetExecuteNum(particle->countNum, 128);
	collider_->SetGPUBuffers(BufferType::CBV, { particle->count, particle->scale, grid->gridConfig });
	collider_->SetGPUBuffers(BufferType::SRV, { grid->particleIndicesForCell, grid->beginIndexForCell, grid->particleCountInCell });
	collider_->SetGPUBuffers(BufferType::UAV, { particle->position, particle->velocity });

#ifdef USE_IMGUI
	configure->Add([&]() {
		ImGui::DragInt("Process Count", &processCount_, 128, 0, maxProcessCount_);
		ImGui::DragInt("Cell Count Per Block", &cellCountPerBlock_, 1, 1, 1024);
		ImGui::DragFloat("Threshold", &commonUpdateData_.x, 0.01f);
		ImGui::DragFloat("Attenuation", &commonUpdateData_.y, 0.01f);
		ImGui::DragFloat("Floor", &commonUpdateData_.z, 0.1f, 0.0f);
		ImGui::DragFloat3("Gravity", (float*)&gravityData_, 0.01f);

		}, "Common Update");
#endif

	uint32_t seed = seedGenerator_(randomEngine_);
	seed_->CopyBuffer(&seed, sizeof(uint32_t));
}

void Sand::Processor::Initialize(SHEngine::ICommandContext* cc, Vector3 pos, float radius) {
	Vector4 ballSphereBuffer = { pos.x, pos.y, pos.z, radius };
	ballShapeData_->CopyBuffer(&ballSphereBuffer, sizeof(float) * 4);
	ballShape_->SetExecuteNum(processCount_, 128);
	ballShape_->Execute(cc);
}

void Sand::Processor::Update(SHEngine::ICommandContext* cc, float deltaTime) {
	//値の更新
	uint32_t seed = seedGenerator_(randomEngine_);
	seed_->CopyBuffer(&seed, sizeof(uint32_t));
	deltaTime_->CopyBuffer(&deltaTime, sizeof(float));

	gravity_->CopyBuffer(&gravityData_, sizeof(Vector3));
	forceData_->CopyBuffer(&commonUpdateData_, sizeof(Vector3));
	cellCountPerBlockBuffer_->CopyBuffer(&cellCountPerBlock_, sizeof(uint32_t));

	//実行数の調整
	acceptForce_->SetExecuteNum(processCount_, 128);
	counter_->SetExecuteNum(processCount_ / 8 + 1, 128);
	assignCell_->SetExecuteNum(processCount_ / 8 + 1, 128);
	collider_->SetExecuteNum(processCount_, 128);

	frameInitialize_->Execute(cc);

	acceptForce_->Execute(cc);

	counter_->Execute(cc);
	splitBlock_->Execute(cc);

	cc->BeginTimeStamp("PrefixSum");
	prefixSum_->Execute(cc);
	cc->EndTimeStamp();

	assignCell_->Execute(cc);
	for (int i = 0; i < 1; ++i) {
		collider_->Execute(cc);
	}

	timeConfig_.prefixSum = cc->GetTimeStampResult("PrefixSum");
}

void Sand::Processor::Save(BinaryManager* bin) const {
	bin->Register(commonUpdateData_);
	bin->Register(gravityData_);
	bin->Register(processCount_);
	bin->Register(cellCountPerBlock_);
}

void Sand::Processor::Load(BinaryManager* bin) {
	bin->Reverse(commonUpdateData_);
	bin->Reverse(gravityData_);
	bin->Reverse(processCount_);
	bin->Reverse(cellCountPerBlock_);
}
