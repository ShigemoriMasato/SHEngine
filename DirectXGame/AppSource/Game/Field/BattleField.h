#pragma once
#include <Particle/ParticleSystem.h>
#include <Render/Screen/Display.h>
#include <Tool/ImGui/ImGuiWindowManager.h>

class BattleField {
public:

	BattleField(ParticleSystem* particleSystem, SHEngine::TextureManager* textureManager, ImGuiWindowManager* imguiManager, ICC* icc);

	void Initialize();
	void Update(float deltaTime, ICC* icc);
	void Draw(DCC* dcc);

private:

	Particle::Emitter* emitter_;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	SHEngine::GPUBuffer* scale_ = nullptr;
	SHEngine::GPUBuffer* textureConfig_ = nullptr;
	SHEngine::GPUBuffer* texture_ = nullptr;
	SHEngine::GPUBuffer* uvs_ = nullptr;

	std::unique_ptr<SHEngine::Screen::Display> display_ = nullptr;

	const uint32_t textureSize_ = 1024;

	Transform transform_ = {};
};
