#pragma once
#include <Particle/ParticleSystem.h>
#include <Render/Screen/Display.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <SHEngine.h>
#include <random>

#include "ShootingStar/ShootingStar.h"

struct ExplosionConfig {
	Vector2 impactPos;
	float radius;
	float scatter;

	float minScatter;
	float heightRange;
	float heightOffset;
	float randForce;
	
	float randRange;
	float randHeightRange;

	uint32_t seed;

	void DrawImGui();
	void Save(BinaryManager& manager);
	void Load(BinaryManager& manager);
};

class BattleField {
public:

	BattleField(ParticleSystem* particleSystem, SHEngine::Engine* engine, ImGuiWindowManager* imguiManager, ICC* icc);
	~BattleField();

	void Initialize();
	void Update(float deltaTime);
	void Draw(DCC* dcc);

	void ApplyTexture(ICC* icc);

	void Explosion(const ExplosionConfig& config);

	ExplosionConfig GetTestConfig() const { return testConfig_; }

private: // items

	std::unique_ptr<ShootingStar> shootingStar_ = nullptr;

private:

	void Save();
	void Load();

	SHEngine::Engine* engine_ = nullptr;

	Camera camera_ = {};

	Particle::Emitter* emitter_;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	SHEngine::GPUBuffer* scale_ = nullptr;
	SHEngine::GPUBuffer* heightScale_ = nullptr;
	SHEngine::GPUBuffer* textureConfig_ = nullptr;
	SHEngine::GPUBuffer* texture_ = nullptr;
	SHEngine::GPUBuffer* heightTexture_ = nullptr;
	SHEngine::GPUBuffer* uvs_ = nullptr;
	SHEngine::GPUBuffer* velocities_ = nullptr;
	SHEngine::GPUBuffer* explosionConfig_ = nullptr;
	SHEngine::GPUBuffer* deltaTime_ = nullptr;

	SHEngine::GPUBuffer* debugParamBuffer_ = nullptr;
	int debugParam_ = 0;
	SHEngine::GPUBuffer* testTexture_[2] = {};
	int testTextureIndex_ = 0;

	ExplosionConfig testConfig_ = {};

	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	std::unique_ptr<SHEngine::Screen::Display> display_ = nullptr;

	const uint32_t textureSize_ = 1024;

	Transform transform_ = {};

	std::mt19937 randomEngine_ = std::mt19937(std::random_device()());
	std::uniform_int_distribution<uint32_t> seedGenerator_ = std::uniform_int_distribution<uint32_t>(0, UINT32_MAX);
};
