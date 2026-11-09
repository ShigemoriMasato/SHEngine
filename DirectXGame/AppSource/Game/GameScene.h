#pragma once
#include <Scene/IScene.h>
#include <Particle/ParticleSystem.h>
#include <Camera/DebugCamera.h>
#include <Game/Field/BattleField.h>
#include <Game/Boss/Beam/ExplosionBeam.h>

class GameScene : public IScene {
public:

	~GameScene() override;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	ImGuiWindowManager imguiManager_;

	// 1,048,576 slots for BattleField plus 4,194,304 for ExplosionBeam.
	ParticleSystem particleSystem_ = { 5242880, true };
	
	std::unique_ptr<BattleField> battleField_ = nullptr;
	std::unique_ptr<ExplosionBeam> explosionBeam_ = nullptr;

	ExplosionBeamConfig beamConfig_;
};
