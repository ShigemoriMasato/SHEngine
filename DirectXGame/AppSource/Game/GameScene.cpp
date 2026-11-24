#include "GameScene.h"
#include <algorithm>

GameScene::~GameScene() {
	// ゲーム(調整値の保存を含む)を、参照先のParticleSystem等より先に破棄する
	shootingGame_.reset();
}

void GameScene::Initialize() {
	particleSystem_.Initialize(directContext_);

	const auto size = commonData_->display->GetTextureData()->GetSize();
	const Vector2 screenSize = { static_cast<float>(size.first), static_cast<float>(size.second) };

	shootingGame_ = std::make_unique<ShootingGame>(engine_, commonData_->keyManager.get(), &particleSystem_, &imguiManager_);
	imguiManager_.PushGroup("Game");
	shootingGame_->Initialize(screenSize);
	imguiManager_.PopGroup();

	// パーティクルの描画にもゲームカメラを使う
	shootingGame_->ResetCameras();

	auto displaySize = commonData_->display->GetTextureData()->GetSize();
	postEffectContext_.Initialize(displaySize.first, displaySize.second);
	bloomEffect_.Initialize(*engine_);
	bloomEffect_.SetMaskTexture(commonData_->display->GetTextureAllData()[1]->GetHandle());
	imguiManager_.Add([&]() {
		bloom_.DrawImGui();
	}, "Bloom");

	imguiManager_.Add([&]() {
		deltaTimeManager_.DrawImGui();
	}, "DeltaTime");

	battleField_ = std::make_unique<BattleField>(&particleSystem_, engine_, &imguiManager_, computeContext_);
	battleField_->Initialize();
}

std::unique_ptr<IScene> GameScene::Update() {
	commonData_->keyManager->Update();
	deltaTimeManager_.Update(engine_->GetDeltaTime());

	auto key = commonData_->keyManager->GetKeyStates();
	const float deltaTime = deltaTimeManager_.GetDeltaTime();

	bloomEffect_.SetBloom(bloom_);

	shootingGame_->Update(deltaTime);
	if (shootingGame_->UseSpecial()) {
		deltaTimeManager_.Boot("Special");
	}

	particleSystem_.Update(computeContext_);
	
	battleField_->Update(deltaTime);
	battleField_->Draw(directContext_);

	auto fieldTextureDrawed = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fieldTextureDrawed);

	battleField_->ApplyTexture(computeContext_);

	auto endApply = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(endApply);

	shootingGame_->UpdateRenderers(deltaTime);

	if (key[Key::One]) {
		battleField_->Explosion(battleField_->GetTestConfig());
	}
	if (key[Key::Two]) {
		battleField_->Initialize();
	}

#ifdef SH_DEBUG
	if (key[Key::Escape]) {
		commonData_->shutDown_ = true;
	}
#endif

	return std::unique_ptr<IScene>();
}

void GameScene::Draw() {
	DefaultPreDraw();
	auto display = commonData_->display.get();

	particleSystem_.Draw(directContext_);

	auto particleDraw = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(particleDraw);

	directContext_->SetRenderTarget(display, false, true);

	//================
	// その他描画処理
	//================

	shootingGame_->Draw();

	//================
	// その他描画処理
	//================

	shootingGame_->DrawImGui();

	imguiManager_.DrawImGui();

	display->ToTexture(directContext_);

	postEffectContext_.Begin(display);
	bloomEffect_.Apply(&postEffectContext_);
	postEffectContext_.End();

	DefaultPostDraw();
}
