#include "FinScene.h"
#include <Utility/Easing.h>

FinScene::FinScene(SHEngine::Engine* engine) {
	engine_ = engine;

	SideBox::StaticInitialize(engine_);

	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		auto& sideBox = sideBoxes_.emplace_back();
	}

	if (!display_) {
		display_ = std::make_unique<SHEngine::Screen::Display>();
		display_->Initialize(1280, 720, "FinScene");
		display_->AddRenderTarget(engine->GetTextureManager(), 0x000000ff);
	}

	backGround_.Initialize(engine->GetTextureManager());

	backGroundConfig_.dcc = engine->GetDirectCommandContext();
	backGroundConfig_.jobs_ = static_cast<uint32_t>(PostEffectJob::Fade);
	backGroundConfig_.origin = display_.get();

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();
}

void FinScene::Initialize(Vector4 fadeColor, std::string title, Vector4 titleColor) {
	const auto planeMesh = engine_->GetModelManager()->GetModelData(SHEngine::TestModel::Plane)->meshes.front();
	const std::string useFont = "851Gkktt_005.ttf";

	titleText_.Initialize(planeMesh, useFont, 64, "Title");
	titleText_.SetText(title);
	titleText_.SetColor(titleColor);
	titleText_.IsUI(true);

	SideBox::Config initialConfig;
	initialConfig.scale = { 0, 0 };

	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		sideBoxes_[i].Initialize(textMap_[i]);
		sideBoxes_[i].SetConfig(initialConfig);
	}

	fade_.color = fadeColor;
}

FinSceneUI FinScene::Update(float deltaTime, Vector2 mousePos) {
	if (timer_ <= fadeTime_ + setupTime_) {
		timer_ += deltaTime;
	}

	FadeProcess();

	if (timer_ > fadeTime_) {
		UIUpdate(deltaTime);
	}

	if (timer_ > fadeTime_ + setupTime_) {
		UIUpdate(deltaTime);
	}

	titleText_.Update(orthoCamera_.GetVPMatrix());
	for (auto& sideBox : sideBoxes_) {
		sideBox.Update(deltaTime, &orthoCamera_, mousePos);
	}

	return currentBox_;
}

void FinScene::DrawReady(DCC* dcc) {
	backGround_.Draw(backGroundConfig_);
	titleText_.Draw(dcc);
	for (auto& sideBox : sideBoxes_) {
		sideBox.Draw(dcc);
	}
}

void FinScene::Draw(DCC* dcc) {
}

void FinScene::DrawImGui() {
}

void FinScene::FadeProcess() {
	fade_.t = std::clamp(timer_ / fadeTime_, 0.0f, 1.0f);
	backGround_.CopyBuffer(PostEffectJob::Fade, &fade_);
}

void FinScene::UISetup(float deltaTime) {
	SideBox::Config config;
	float t = std::clamp((timer_ - fadeTime_) / setupTime_, 0.0f, 1.0f);
	Vector2 initPosition = { -360.0f, 0.0f };
	Vector2 currentPosition = lerp(initPosition, offset_, t, EaseType::EaseOutCubic);
	
	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		float yOffset = 
		sideBoxes_[i].SetConfig(config);
	}
}

void FinScene::UIUpdate(float deltaTime) {
}
