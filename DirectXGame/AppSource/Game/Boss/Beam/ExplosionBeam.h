#pragma once

#include <Particle/ParticleSystem.h>
#include <SHEngine.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <string>

class BinaryManager;

struct ExplosionBeamConfig {
	Vector3 startPoint{ 10.0f, 10.0f, 0.0f };
	Vector3 endPoint{ -10.0f, 10.0f, 0.0f };
	float emissionRate = 1'000'000.0f;
	float particleSpeed = 5.0f;
	// 正の値で終点に向かって細く、負の値で太くする。-1～1が目安。
	float convergenceRate = 0.0f;
	// 射出位置に加えるランダムなXYZオフセットの最大値（ワールド単位）。
	float randomness = 0.0f;
	float innerRadius = 0.05f;
	float outerRadius = 0.5f;
	float particleRadius = 0.05f;
	Vector4 colorMultiplier{ 1.0f, 1.0f, 1.0f, 1.0f };

	void DrawImGui();
	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);
};

class ExplosionBeam {
public:
	ExplosionBeam(ParticleSystem* particleSystem, SHEngine::Engine* engine, ICC* icc,
		uint32_t particleCount, std::string texturePath);

	void Update(float deltaTime);
	
	void SetConfig(const ExplosionBeamConfig& config);

private:
	struct alignas(16) BeamGpuConfig {
		Vector3 startPoint{};
		float emissionRate = 0.0f;
		Vector3 endPoint{};
		float innerRadius = 0.0f;
		Vector4 colorMultiplier{};
		float outerRadius = 0.0f;
		float particleRadius = 0.0f;
		uint32_t emissionCount = 0;
		uint32_t emissionCursor = 0;
		uint32_t seed = 0;
		float particleSpeed = 0.0f;
		float deltaTime = 0.0f;
		float convergenceRate = 0.0f;
		float randomness = 0.0f;
		Vector3 padding{};
	};
	static_assert(sizeof(BeamGpuConfig) == 96, "BeamGpuConfig must match the HLSL constant buffer layout");

	void BindUpdateProcess();

	Particle::Emitter* emitter_ = nullptr;
	SHEngine::Engine* engine_ = nullptr;
	std::unique_ptr<SHEngine::BufferContainer> container_;
	SHEngine::GPUBuffer* configBuffer_ = nullptr;
	SHEngine::GPUBuffer* colorTexture_ = nullptr;
	ExplosionBeamConfig config_;
	uint32_t seed_ = 1;
	uint32_t emissionCursor_ = 0;
	float emissionFraction_ = 0.0f;
	uint32_t updateProcessID_ = 0;

	const uint32_t particleCount_ = 0;
};
