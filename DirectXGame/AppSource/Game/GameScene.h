#pragma once
#include <Scene/IScene.h>
#include <Particle/ParticleSystem.h>
#include <Render/PostEffect/PostEffectContext.h>
#include <Render/PostEffect/BloomEffect.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <Common/DeltaTime/DeltaTimeManager.h>

#include <Game/ShootingGame/ShootingGame.h>

/// @brief ゲームシーン。入力更新・パーティクル・ポストエフェクト・ImGuiなどの共通処理を行い、
/// ゲーム本体(Player・Enemy・Weapon・Bullet・Fieldなど)はShootingGameへ委譲する。
class GameScene : public IScene {
public:

	~GameScene() override;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	// 1フレームのdeltaTimeの上限(秒)。ヒッチ時の大移動を防ぐ
	static constexpr float kMaxDeltaTime = 1.0f / 20.0f;

	ImGuiWindowManager imguiManager_;
	DeltaTimeManager deltaTimeManager_;

	// 1,048,576 slots for BattleField plus 4,194,304 for ExplosionBeam.
	ParticleSystem particleSystem_ = { 5242880, true };
	std::unique_ptr<BattleField> battleField_ = nullptr;

	PostEffectContext postEffectContext_;
	BloomEffect bloomEffect_;
	Bloom bloom_;

	

	// ゲーム本体。ParticleSystem・ImGuiWindowManagerを非所有で参照するため、それらより後に宣言する(先に破棄される)
	std::unique_ptr<ShootingGame> shootingGame_ = nullptr;
};
