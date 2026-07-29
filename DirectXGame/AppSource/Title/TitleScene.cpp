#include "TitleScene.h"
#include <Game/GameScene.h>

TitleScene::TitleScene() {
	waterWave_ = std::make_unique<WaterWave>();
	camera_ = std::make_unique<Camera>();
}

TitleScene::~TitleScene() {
	Save();
}

void TitleScene::Initialize() {
	auto model = modelManager_->LoadModel("WaterPlane");
	waterWave_->Initialize(model->meshes.front(), camera_.get());

	postEffect_.Initialize(textureManager_);
	peConfig_.origin = commonData_->display.get();
	peConfig_.dcc = directContext_;
	peConfig_.jobs = uint32_t(PostEffectJob::Fade);
	fade_.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	fade_.t = 0.0f;

	camera_->SetProjectionMatrix(PerspectiveFovDesc());
	camera_->SetPosition({ 0.0f, 0.0f, -14.0f });
	camera_->MakeMatrix();

	effect_ = std::make_unique<Effect>();
	effect_->Initialize(engine_, 10000000);

	polygonEmitter_ = std::make_unique<PolygonEmitter>();
	effect_->AddEmitter(polygonEmitter_.get());

	isFadeOut_ = false;

	model = modelManager_->LoadModel("Title");
	polygonConfig_ = polygonEmitter_->AddPolygon(model->meshes);
	Load();
	polygonConfig_.speed = 0.05f;
}

std::unique_ptr<IScene> TitleScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	input_->Update();
	commonData_->keyManager->Update();
	camera_->MakeMatrix();
	waterWave_->Update(deltaTime);

	polygonEmitter_->SetConfig(polygonConfig_);
	effect_->Update(camera_.get(), deltaTime);

	auto key = commonData_->keyManager->GetKeyStates();

	if (key.at(Key::Correct)) {
		isFadeOut_ = true;
	}

	if (isFadeOut_) {
		timer_ += deltaTime;
		fade_.t = std::clamp(timer_ / fadeTime_, 0.0f, 1.0f);
		polygonConfig_.speed = 3.f;
		//フェード完了から少し待つ
		if (timer_ > fadeTime_ + 0.2f) {
			return std::make_unique<GameScene>();
		}
	}

	postEffect_.CopyBuffer(PostEffectJob::Fade, fade_);

	return nullptr;
}

void TitleScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(display);

	waterWave_->Draw(directContext_);

	effect_->Draw();

	display->ToTexture(directContext_);

#ifdef SH_RELEASE

	peConfig_.output = window->GetCurrentDisplay();
	postEffect_.Draw(peConfig_);
	directContext_->SetRenderTarget(window->GetCurrentDisplay(), false);

#else

	peConfig_.output = nullptr;
	postEffect_.Draw(peConfig_);
	directContext_->SetRenderTarget(window);

#endif

	//ここ以外で記述する場合、ifdefを忘れないようにすること
#ifdef USE_IMGUI
	display->DrawImGui();
	waterWave_->DrawImGui();
	camera_->DrawImGui();

	ImGui::Begin("PolygonEmitter");
	polygonConfig_.DrawImGui();
	ImGui::End();
#endif

	engine_->DrawImGui();
	window->ToPresent(directContext_);

}

void TitleScene::Save() {
	BinaryManager bin;
	polygonConfig_.Save(bin);
	bin.Write(dataFileName_);
}

void TitleScene::Load() {
	BinaryManager bin;
	if (!bin.Boot(dataFileName_)) {
		return;
	}
	polygonConfig_.Load(bin);
}
