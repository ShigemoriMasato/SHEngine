#pragma once
#include <Scene/IScene.h>
#include <Particle/ParticleSystem.h>
#include <Camera/DebugCamera.h>
#include <Game/Field/BattleField.h>

class GameScene : public IScene {
public:

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	ImGuiWindowManager imguiManager_;

	ParticleSystem particleSystem_ = { 16777216 };
	
	std::unique_ptr<BattleField> battleField_ = nullptr;
};
