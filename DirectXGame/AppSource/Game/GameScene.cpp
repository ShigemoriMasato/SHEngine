#include "GameScene.h"
#include <Tool/Binary/BinaryManager.h>

GameScene::~GameScene() {
	if (explosionBeam_) {
		BinaryManager manager;
		beamConfig_.Save(manager);
		manager.Write("ExplosionBeam.bin");
	}
}

void GameScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	particleSystem_.Initialize(directContext_);
	particleSystem_.SetCamera(debugCamera_.get());

	battleField_ = std::make_unique<BattleField>(&particleSystem_, engine_, &imguiManager_, computeContext_);
	battleField_->Initialize();

	BinaryManager beamConfigFile;
	if (beamConfigFile.Boot("ExplosionBeam.bin")) {
		beamConfig_.Load(beamConfigFile);
	}
	//explosionBeam_ = std::make_unique<ExplosionBeam>(&particleSystem_, engine_, computeContext_, 4000000, "Texture/uvChecker.png");
	//explosionBeam_->SetConfig(beamConfig_);
	imguiManager_.Add([this]() {
		beamConfig_.DrawImGui();
	}, "ExplosionBeam");

	postEffectContext_.Initialize(1280, 720);
	bloomEffect_.Initialize(*engine_);
	bloomEffect_.SetMaskTexture(commonData_->display->GetTextureAllData()[1]->GetHandle());
}

std::unique_ptr<IScene> GameScene::Update() {
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();

	debugCamera_->Update();
	float deltaTime = engine_->GetDeltaTime();

	battleField_->Update(deltaTime);

	//explosionBeam_->SetConfig(beamConfig_);
	//explosionBeam_->Update(deltaTime);
	particleSystem_.Update(computeContext_);
	battleField_->Draw(directContext_);

	auto fieldTextureDrawed = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fieldTextureDrawed);

	battleField_->ApplyTexture(computeContext_);

	auto endApply = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(endApply);

	if (key[Key::One]) {
		battleField_->Explosion(battleField_->GetTestConfig());
	}
	if (key[Key::Two]) {
		battleField_->Initialize();
	}

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



	//================
	// その他描画処理
	//================

#ifdef USE_IMGUI
	ImGui::Begin("Bloom");
	bloomEffect_.DrawImGui();
	ImGui::End();
#endif

	imguiManager_.DrawImGui();

	display->ToTexture(directContext_);

	postEffectContext_.Begin(display);
	bloomEffect_.Apply(&postEffectContext_);
	postEffectContext_.End();

	DefaultPostDraw();
}
