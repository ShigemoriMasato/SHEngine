#include "TitleScene.h"
#include <Game/GameScene.h>

TitleScene::TitleScene() {
	waterWave_ = std::make_unique<WaterWave>();
	camera_ = std::make_unique<Camera>();
}

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {
	auto model = modelManager_->LoadModel("WaterPlane");
	waterWave_->Initialize(model->meshes.front(), camera_.get());

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, true);		//描画だけする
	postEffectConfig_.origin = commonData_->display.get();
	postEffectConfig_.jobs_ = uint32_t(PostEffectJob::None);

	camera_->SetProjectionMatrix(PerspectiveFovDesc());
	camera_->SetPosition({ 0.0f, 0.0f, -14.0f });
	camera_->MakeMatrix();
}

std::unique_ptr<IScene> TitleScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	input_->Update();
	commonData_->keyManager->Update();

	camera_->MakeMatrix();

	waterWave_->Update(deltaTime);

	auto key = commonData_->keyManager->GetKeyStates();

	if (key.at(Key::Hold) || key.at(Key::HardDrop)) {
		return std::make_unique<GameScene>();
	}

	return nullptr;
}

void TitleScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(display);
	
	waterWave_->Draw(directContext_);

	display->ToTexture(directContext_);

#ifdef SH_RELEASE

	postEffectConfig_.dcc = directContext_;
	postEffect_->Draw(postEffectConfig_);
	directContext_->SetRenderTarget(window->GetCurrentDisplay(), false);

#else

	directContext_->SetRenderTarget(window);

#endif

	//ここ以外で記述する場合、ifdefを忘れないようにすること
#ifdef USE_IMGUI
	display->DrawImGui();
	waterWave_->DrawImGui();
	camera_->DrawImGui();
#endif

	engine_->DrawImGui();
	window->ToPresent(directContext_);

}
