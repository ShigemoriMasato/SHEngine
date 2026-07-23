#include "FinScene.h"
#include <Utility/Easing.h>

FinScene::FinScene(SHEngine::Engine* engine, SHEngine::Screen::Display* display) {
	engine_ = engine;
	display_ = display;

	SideBox::StaticInitialize(engine_);

	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		auto& sideBox = sideBoxes_.emplace_back();
	}

	backGround_.Initialize(engine->GetTextureManager());

	backGroundConfig_.dcc = engine->GetDirectCommandContext();
	backGroundConfig_.jobs_ = static_cast<uint32_t>(PostEffectJob::Fade);
	backGroundConfig_.origin = display_;

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	lastCopy_.Initialize(engine->GetTextureManager(), true);
	lastCopyConfig_.dcc = engine->GetDirectCommandContext();
	lastCopyConfig_.jobs_ = static_cast<uint32_t>(PostEffectJob::None);
	lastCopyConfig_.origin = display_;
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

	isInitialized_ = true;
}

FinSceneUI FinScene::Update(float deltaTime, Vector2 mousePos) {
	if (!isInitialized_) {
		return FinSceneUI::None;
	}

	if (timer_ <= fadeTime_ + setupTime_) {
		timer_ += deltaTime;
	}

	FadeProcess();

	if (timer_ > fadeTime_) {
		UISetup(deltaTime);
	}

	if (timer_ > fadeTime_ + setupTime_) {
		UIUpdate(deltaTime);
	}

	titleText_.Update(orthoCamera_.GetVPMatrix());

	currentBox_ = FinSceneUI::None;
	for (uint32_t i = 0; i < sideBoxes_.size(); ++i) {
		if (sideBoxes_[i].Update(deltaTime, &orthoCamera_, mousePos)) {
			currentBox_ = static_cast<FinSceneUI>(i);
		}
	}

	return currentBox_;
}

void FinScene::DrawReady(DCC* dcc) {
	if (!isInitialized_) {
		return;
	}

	backGround_.Draw(backGroundConfig_);
	dcc->SetRenderTarget(display_);

	titleText_.Draw(dcc);
	for (auto& sideBox : sideBoxes_) {
		sideBox.Draw(dcc);
	}
}

void FinScene::Draw(DCC* dcc) {
	if (!isInitialized_) {
		return;
	}

	display_->ToTexture(dcc);

	lastCopyConfig_.output = dcc->GetRenderTarget();
	lastCopy_.Draw(lastCopyConfig_);
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
	float top = offset_.y + margin_ * float(int(FinSceneUI::Count) - 1) / 2.0f;
	
	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		float yOffset = top - margin_ * float(i);
		config.position = { currentPosition.x, yOffset };
		sideBoxes_[i].SetConfig(config);
	}
}

void FinScene::UIUpdate(float deltaTime) {
}
