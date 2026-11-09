#include "ExplosionBeam.h"

#include <Tool/Binary/BinaryManager.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>
#include <cstring>

void ExplosionBeamConfig::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::PushID("ExplosionBeamConfig");
	ImGui::DragFloat3("Start Point", &startPoint.x, 0.1f);
	ImGui::DragFloat3("End Point", &endPoint.x, 0.1f);
	ImGui::DragFloat("Emission Rate", &emissionRate, 1000.0f, 0.0f, 10000000.0f, "%.0f / sec");
	ImGui::DragFloat("Particle Speed", &particleSpeed, 0.1f, 0.0f, 10000.0f);
	ImGui::DragFloat("Convergence Rate", &convergenceRate, 0.01f, -1.0f, 1.0f, "%.2f");
	ImGui::DragFloat("Spawn Randomness", &randomness, 0.01f, 0.0f, 100.0f, "%.2f");
	ImGui::DragFloat("Inner Radius", &innerRadius, 0.01f, 0.0f, 10000.0f);
	ImGui::DragFloat("Outer Radius", &outerRadius, 0.01f, 0.0f, 10000.0f);
	ImGui::DragFloat("Particle Radius", &particleRadius, 0.01f, 0.0f, 10000.0f);
	ImGui::ColorEdit4("Color Multiplier", &colorMultiplier.x);
	ImGui::PopID();
#endif
}

void ExplosionBeamConfig::Save(BinaryManager& manager) const {
	manager.Register(startPoint);
	manager.Register(endPoint);
	manager.Register(emissionRate);
	manager.Register(particleSpeed);
	manager.Register(innerRadius);
	manager.Register(outerRadius);
	manager.Register(particleRadius);
	manager.Register(colorMultiplier);
	manager.Register(convergenceRate);
	manager.Register(randomness);
}

void ExplosionBeamConfig::Load(BinaryManager& manager) {
	manager.Reverse(startPoint);
	manager.Reverse(endPoint);
	manager.Reverse(emissionRate);
	manager.Reverse(particleSpeed);
	manager.Reverse(innerRadius);
	manager.Reverse(outerRadius);
	manager.Reverse(particleRadius);
	manager.Reverse(colorMultiplier);
	manager.Reverse(convergenceRate);
	manager.Reverse(randomness);

	emissionRate = std::max(0.0f, emissionRate);
	particleSpeed = std::max(0.0f, particleSpeed);
	convergenceRate = std::clamp(convergenceRate, -1.0f, 1.0f);
	randomness = std::max(0.0f, randomness);
	innerRadius = std::max(0.0f, innerRadius);
	outerRadius = std::max(innerRadius, outerRadius);
	particleRadius = std::max(0.0f, particleRadius);
}

ExplosionBeam::ExplosionBeam(ParticleSystem* particleSystem, SHEngine::Engine* engine, ICC* icc, uint32_t particleCount, std::string texturePath)
	: engine_(engine), particleCount_(std::max(1u, particleCount)) {
	emitter_ = particleSystem->CreateEmitter(particleCount_, icc);
	container_ = std::make_unique<SHEngine::BufferContainer>();
	configBuffer_ = container_->Create(BufferType::CBV, sizeof(BeamGpuConfig));
	colorTexture_ = container_->Create(engine_->GetTextureManager()->GetTextureData(texturePath));

	auto initialize = emitter_->AddTriggerProcess("ForGame/Beam/Initialize");
	initialize->SetExecuteNum(static_cast<int>(particleCount_), 128);
	BindUpdateProcess();

	std::random_device randomDevice;
	seed_ = randomDevice();
}

void ExplosionBeam::BindUpdateProcess() {
	auto process = emitter_->AddProcess("ForGame/Beam/EmitUpdate", &updateProcessID_);
	process->SetGPUBuffer(BufferType::CBV, configBuffer_);
	process->SetGPUBuffer(BufferType::SRV, colorTexture_);
	process->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	process->SetExecuteNum(static_cast<int>(particleCount_), 128);
}

void ExplosionBeam::Update(float deltaTime) {
	const float safeDeltaTime = std::max(deltaTime, 0.0f);
	const double requested = static_cast<double>(std::max(0.0f, config_.emissionRate)) * safeDeltaTime + emissionFraction_;
	const uint64_t emitted = static_cast<uint64_t>(std::floor(requested));
	emissionFraction_ = static_cast<float>(requested - static_cast<double>(emitted));

	BeamGpuConfig gpuConfig{};
	gpuConfig.startPoint = config_.startPoint;
	gpuConfig.endPoint = config_.endPoint;
	gpuConfig.emissionRate = config_.emissionRate;
	gpuConfig.particleSpeed = config_.particleSpeed;
	gpuConfig.innerRadius = config_.innerRadius;
	gpuConfig.outerRadius = config_.outerRadius;
	gpuConfig.particleRadius = config_.particleRadius;
	gpuConfig.colorMultiplier = config_.colorMultiplier;
	gpuConfig.emissionCount = static_cast<uint32_t>(std::min<uint64_t>(emitted, particleCount_));
	gpuConfig.emissionCursor = emissionCursor_;
	gpuConfig.seed = seed_++;
	gpuConfig.deltaTime = safeDeltaTime;
	gpuConfig.convergenceRate = std::clamp(config_.convergenceRate, -1.0f, 1.0f);
	gpuConfig.randomness = std::max(0.0f, config_.randomness);
	configBuffer_->CopyBuffer(&gpuConfig, sizeof(gpuConfig));

	emissionCursor_ = static_cast<uint32_t>((static_cast<uint64_t>(emissionCursor_) + emitted % particleCount_) % particleCount_);
}

void ExplosionBeam::SetConfig(const ExplosionBeamConfig& config) {
	config_ = config;
}
