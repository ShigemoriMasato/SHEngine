#include "GameScene.h"

void GameScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	particleSystem_.Initialize(directContext_);
	particleSystem_.SetCamera(debugCamera_.get());

	battleField_ = std::make_unique<BattleField>(&particleSystem_, engine_, &imguiManager_, computeContext_);
	battleField_->Initialize();
}

std::unique_ptr<IScene> GameScene::Update() {
	debugCamera_->Update();
	float deltaTime = engine_->GetDeltaTime();

	battleField_->Update(deltaTime);

	particleSystem_.Update(computeContext_);
	battleField_->Draw(directContext_);

	auto fieldTextureDrawed = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fieldTextureDrawed);

	battleField_->ApplyTexture(computeContext_);

	auto endApply = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(endApply);

	return std::unique_ptr<IScene>();
}

void GameScene::Draw() {
	DefaultPreDraw();

	particleSystem_.Draw(directContext_);

	auto particleDraw = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(particleDraw);

	directContext_->SetRenderTarget(commonData_->window.get(), false, true);

	//================
	// その他描画処理
	//================



	//================
	// その他描画処理
	//================

	imguiManager_.DrawImGui();

	DefaultPostDraw();
}
