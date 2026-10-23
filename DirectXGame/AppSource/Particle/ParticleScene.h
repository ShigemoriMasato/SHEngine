#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Tool/ImGui/ImGuiWindowManager.h>

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
};
