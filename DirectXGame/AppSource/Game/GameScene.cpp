#include "GameScene.h"

void GameScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	particleSystem_.Initialize(directContext_);
	particleSystem_.SetCamera(debugCamera_.get());

	battleField_ = std::make_unique<BattleField>(&particleSystem_, textureManager_, &imguiManager_, directContext_);
	battleField_->Initialize();
}

std::unique_ptr<IScene> GameScene::Update() {
	debugCamera_->Update(true);
	float deltaTime = engine_->GetDeltaTime();

	battleField_->Update(deltaTime, directContext_);

	// パーティクルには関係なく、パーティクルの描画前にバトルフィールドを描画する必要があるため、例外的にここで描画する
	battleField_->Draw(directContext_);

	particleSystem_.Update(directContext_);

	return std::unique_ptr<IScene>();
}

void GameScene::Draw() {
	DefaultPreDraw();

	particleSystem_.Draw(directContext_);

	imguiManager_.DrawImGui();

	DefaultPostDraw();
}
