#include "SandProcessor.h"
#include <imgui/imgui.h>

Sand::Processor::Processor(SHEngine::BufferContainer* container, Particle* particle, Grid* grid, Configure* configure) : maxProcessCount_(particle->countNum) {
	auto createCO = [&](const std::string& name) {
		auto co = std::make_unique<SHEngine::ComputeObject>("SandSimulator::" + name);
		co->SetShader("Sand/System/" + name + ".CS.hlsl");
		return co;
		};

	ballShape_ = createCO("BallShape");

	deltaTime_ = container->Create(BufferType::CBV, sizeof(float));
	seed_ = container->Create(BufferType::CBV, sizeof(uint32_t));
	ballShapeData_ = container->Create(BufferType::CBV, sizeof(float) * 4);
	gravity_ = container->Create(BufferType::CBV, sizeof(Vector3));
	forceData_ = container->Create(BufferType::CBV, sizeof(Vector3));
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

	collider_ = createCO("Collider");
	collider_->SetExecuteNum(particle->countNum, 128);
	collider_->SetGPUBuffers(BufferType::CBV, { particle->count, particle->scale, grid->gridConfig });
	collider_->SetGPUBuffers(BufferType::SRV, { grid->cellIndices, grid->cellCounts });
	collider_->SetGPUBuffers(BufferType::UAV, { particle->position, particle->force, particle->velocity });

	assignCell_ = createCO("AssignCell");
	assignCell_->SetExecuteNum(particle->countNum / 32 + 1, 128);
	assignCell_->SetGPUBuffers(BufferType::CBV, { particle->count, grid->gridConfig });
	assignCell_->SetGPUBuffers(BufferType::UAV, { particle->position, grid->cellIndices, grid->cellCounts });

#ifdef USE_IMGUI
	configure->Add([&]() {
		ImGui::DragInt("Process Count", &processCount_, 128, 0, maxProcessCount_);
		ImGui::DragFloat("Threshold", &commonUpdateData_.x, 0.01f);
		ImGui::DragFloat("Attenuation", &commonUpdateData_.y, 0.01f);
		ImGui::DragFloat("Floor", &commonUpdateData_.z, 0.1f);
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
	uint32_t seed = seedGenerator_(randomEngine_);
	seed_->CopyBuffer(&seed, sizeof(uint32_t));
	deltaTime_->CopyBuffer(&deltaTime, sizeof(float));

	gravity_->CopyBuffer(&gravityData_, sizeof(Vector3));
	forceData_->CopyBuffer(&commonUpdateData_, sizeof(Vector3));

	assignCell_->SetExecuteNum(processCount_ / 32 + 1, 128);
	acceptForce_->SetExecuteNum(processCount_, 128);
	collider_->SetExecuteNum(processCount_, 128);

	acceptForce_->Execute(cc);
	assignCell_->Execute(cc);
	for (int i = 0; i < 6; ++i) {
		collider_->Execute(cc);
	}
}

void Sand::Processor::Save(BinaryManager* bin) const {
	bin->Register(commonUpdateData_);
	bin->Register(gravityData_);
	bin->Register(processCount_);
}

void Sand::Processor::Load(BinaryManager* bin) {
	bin->Reverse(commonUpdateData_);
	bin->Reverse(gravityData_);
	bin->Reverse(processCount_);
}
