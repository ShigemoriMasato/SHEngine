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
	explosionBeam_ = std::make_unique<ExplosionBeam>(&particleSystem_, engine_, computeContext_, 4000000, "Texture/uvChecker.png");
	explosionBeam_->SetConfig(beamConfig_);
	imguiManager_.Add([this]() {
		beamConfig_.DrawImGui();
	}, "ExplosionBeam");
}

std::unique_ptr<IScene> GameScene::Update() {
	debugCamera_->Update();
	float deltaTime = engine_->GetDeltaTime();

	battleField_->Update(deltaTime);

	explosionBeam_->SetConfig(beamConfig_);
	explosionBeam_->Update(deltaTime);
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
