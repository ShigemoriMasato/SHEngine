#pragma once
#include <Scene/IScene.h>
#include <Game/Tetris/Tetris.h>
#include <Camera/DebugCamera.h>
#include <Render/PostEffect.h>
#include <Game/Camera/GameCamera.h>
#include <Game/Effect/Effect.h>
#include <Game/Effect/Subject/Subject.h>

class GameScene : public IScene {
public:

	GameScene();
	~GameScene() override = default;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	Camera* worldCamera_ = nullptr;
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	std::unique_ptr<Camera> manualCamera_ = nullptr;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;

	std::unique_ptr<KeyCoating> keyCoating_;
	std::unique_ptr<Tetris> tetris_;
	std::unique_ptr<PostEffect> postEffect_;
	std::unique_ptr<Effect> effect_ = nullptr;

	std::unique_ptr<SHEngine::RenderObject> gameOverText = nullptr;

	PostEffectConfig postEffectConfig_{};

	std::unique_ptr<Subject> subject_ = nullptr;
};