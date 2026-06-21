#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Game/Effect/Effect.h>
#include <Game/Effect/Subject/HitEffect.h>
#include <Render/PostEffect.h>

class EffectTestScene : public IScene {
public:

	EffectTestScene() {};

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	DebugCamera camera_;

	std::unique_ptr<Effect> effect_ = nullptr;
	std::unique_ptr<HitEffect> hitEffect_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig peConfig_{};
};
