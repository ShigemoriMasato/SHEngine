#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <Particle/ParticleSystem.h>

class ParticleScene : public IScene {
public:

	~ParticleScene();

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	void Save() const;
	void Load();

	std::unique_ptr<DebugCamera> camera_;

	ImGuiWindowManager imguiManager_;

	ParticleSystem particleSystem_{ 1024 * 1024 * 2 };
	std::unique_ptr<SHEngine::BufferContainer> container_;

	Particle::Emitter* emitter_;

	SHEngine::GPUBuffer* discreteBuffer_ = nullptr;

	Transform transform_;
	Vector2 discrete_;
};
