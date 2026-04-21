#include "WaveParticle.h"
#include <imgui/imgui.h>

void WaveParticle::Initialize(const Pool& pool, const uint32_t id) {
	emitter_ = std::make_unique<SHEngine::ComputeObject>("WaveParticle Emitter");
	update_ = std::make_unique<SHEngine::ComputeObject>("WaveParticle Update");

	container_ = std::make_unique<SHEngine::BufferContainer>();
	auto velocities = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), pool.maxParticleNum); // velocity
	auto lifeTimes = container_->Create(BufferType::SRV_UAV, sizeof(float), pool.maxParticleNum); // lifeTime
	speed_ = container_->Create(BufferType::CBV, sizeof(float), 1); // speed
	seed_ = container_->Create(BufferType::CBV, sizeof(uint32_t), 1); // seed
	lifeTime_ = container_->Create(BufferType::CBV, sizeof(float), 1); // lifeTime
	emitNum_ = container_->Create(BufferType::CBV, sizeof(float), 1); // emitNum
	auto idBuffer = container_->Create(BufferType::CBV, sizeof(uint32_t), 1); // idBuffer
	idBuffer->CopyBuffer(&id, sizeof(uint32_t));

	emitter_->SetShader("Particle/Wave/Emit.CS.hlsl");
	emitter_->SetGPUBuffers(BufferType::UAV, { pool.freeList, pool.freeListIndex, pool.type, pool.position, velocities, lifeTimes });
	emitter_->SetGPUBuffers(BufferType::CBV, { emitNum_, speed_, seed_, lifeTime_, idBuffer });

	update_->SetShader("Particle/Wave/Update.CS.hlsl");

	update_->SetGPUBuffers(BufferType::UAV, { pool.freeList, pool.freeListIndex, pool.position, pool.color, pool.type,
		velocities, lifeTimes });
	update_->SetGPUBuffers(BufferType::CBV, { pool.particleNum, pool.deltaTime, idBuffer, lifeTime_ });
	update_->SetThreadGroupSize(pool.maxParticleNum / 1024);
}

void WaveParticle::Update(CmdObj* compute) {
	emitter_->SetThreadGroupSize(int(config_.emitNum / 1024) + 1);
	emitter_->Execute(compute);
	update_->Execute(compute);
}

void WaveParticle::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("WaveParticle");
	ImGui::DragFloat("Speed", &config_.speed, 0.0f, 10.0f);
	ImGui::DragFloat("Seed", &config_.seed, 0.0f, 100.0f);
	ImGui::DragFloat("LifeTime", &config_.lifeTime, 0.1f, 5.0f);
	ImGui::SliderInt("EmitNum", &config_.emitNum, 1, 10000);
	speed_->CopyBuffer(&config_.speed, sizeof(float));
	seed_->CopyBuffer(&config_.seed, sizeof(float));
	lifeTime_->CopyBuffer(&config_.lifeTime, sizeof(float));
	emitNum_->CopyBuffer(&config_.emitNum, sizeof(int));
	ImGui::End();
#endif
}
