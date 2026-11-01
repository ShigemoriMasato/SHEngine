#pragma once
#include <Particle/ParticleSystem.h>
#include <Render/Screen/Display.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <SHEngine.h>

#include "ShootingStar/ShootingStar.h"

class BattleField {
public:

	BattleField(ParticleSystem* particleSystem, SHEngine::Engine* engine, ImGuiWindowManager* imguiManager, ICC* icc);
	~BattleField();

	void Initialize();
	void Update(float deltaTime);
	void Draw(DCC* dcc);

	void ApplyTexture(ICC* icc);

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

	SHEngine::GPUBuffer* debugParamBuffer_ = nullptr;
	int debugParam_ = 0;
	SHEngine::GPUBuffer* testTexture_[2] = {};
	int testTextureIndex_ = 0;

	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	std::unique_ptr<SHEngine::Screen::Display> display_ = nullptr;

	const uint32_t textureSize_ = 1024;

	Transform transform_ = {};
};
