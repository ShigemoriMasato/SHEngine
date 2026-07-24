#include "FinScene.h"
#include <Utility/Easing.h>

FinScene::FinScene(SHEngine::Engine* engine) {
	engine_ = engine;

	SideBox::StaticInitialize(engine_);

	sideBoxes_.resize(static_cast<uint32_t>(FinSceneUI::Count));

	backGround_.Initialize(engine->GetTextureManager());

	backGroundConfig_.dcc = engine->GetDirectCommandContext();
	backGroundConfig_.jobs_ = static_cast<uint32_t>(PostEffectJob::Fade);

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();
}

void FinScene::Initialize(Vector4 fadeColor, std::string title, Vector4 titleColor) {
	const auto planeMesh = engine_->GetModelManager()->GetModelData(SHEngine::TestModel::Plane)->meshes.front();
	const std::string useFont = "851Gkktt_005.ttf";

	titleText_.Initialize(planeMesh, useFont, 128, "Title");
	titleText_.SetText(title);
	titleText_.SetColor(titleColor);
	titleText_.SetIsUI(true);

	SideBox::Config initialConfig;
	initialConfig.scale = { 0, 0 };

	for (uint32_t i = 0; i < static_cast<uint32_t>(FinSceneUI::Count); ++i) {
		sideBoxes_[i].Initialize(textMap_[i]);
		sideBoxes_[i].SetConfig(initialConfig);
	}

	fade_.color = fadeColor;
	fade_.t = 0.0f;

	colorMap_.resize(static_cast<uint32_t>(FinSceneUI::Count));

	isInitialized_ = true;
}

FinSceneUI FinScene::Update(float deltaTime, Vector2 mousePos, std::unordered_map<Key, bool> keys) {
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

	UIUpdate(deltaTime, mousePos, keys);
	
	titleText_.SetTransform({ {titleScale_.x, titleScale_.y, 1.0f}, {}, {titlePos_.x, titlePos_.y, 0.0f} });
	titleText_.Update(orthoCamera_.GetVPMatrix());

	return currentBox_;
}

void FinScene::Draw(DCC* dcc) {
	if (!isInitialized_) {
		return;
	}

	auto renderTarget = dcc->GetRenderTarget();

	backGroundConfig_.origin = renderTarget;
	backGround_.Draw(backGroundConfig_);

	//RenderTargetを戻す
	dcc->SetRenderTarget(renderTarget, false);

	titleText_.Draw(dcc);
	for (auto& sideBox : sideBoxes_) {
		sideBox.Draw(dcc);
	}

	SideBox::DrawImGui();
}

void FinScene::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("FinScene");

	if (ImGui::TreeNode("Timer")) {
		ImGui::DragFloat("FadeTime", &fadeTime_, 0.01f);
		ImGui::DragFloat("SetupTime", &setupTime_, 0.01f);
		if (ImGui::Button("Reset")) {
			timer_ = 0.0f;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Title")) {
		ImGui::DragFloat2("Position", &titlePos_.x, 0.01f);
		ImGui::DragFloat2("Scale", &titleScale_.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SideBoxes")) {
		ImGui::DragFloat("Margin", &margin_, 0.01f);
		ImGui::DragFloat2("Offset", &offset_.x, 0.01f);
		ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Other")) {
		for (int i = 0; i < colorMap_.size(); ++i) {
			ImGui::ColorEdit4(("Color" + std::to_string(i)).c_str(), &colorMap_[i].x);
		}
		ImGui::TreePop();
	}

	ImGui::End();

#endif
}

void FinScene::Save(BinaryManager& bin) {
	bin.Register(&fadeTime_);
	bin.Register(&setupTime_);
	bin.Register(&titlePos_);
	bin.Register(&titleScale_);
	bin.Register(&margin_);
	bin.Register(&offset_);
	bin.Register(&scale_);
	uint32_t colorMapSize = static_cast<uint32_t>(colorMap_.size());
	bin.Register(&colorMapSize);
	for (uint32_t i = 0; i < colorMapSize; ++i) {
		bin.Register(&colorMap_[i]);
	}
}

void FinScene::Load(BinaryManager& bin) {
	fadeTime_ = bin.Reverse<float>();
	setupTime_ = bin.Reverse<float>();
	titlePos_ = bin.Reverse<Vector2>();
	titleScale_ = bin.Reverse<Vector2>();
	margin_ = bin.Reverse<float>();
	offset_ = bin.Reverse<Vector2>();
	scale_ = bin.Reverse<Vector2>();
	uint32_t colorMapSize = bin.Reverse<uint32_t>();
	colorMap_.resize(colorMapSize);
	for (uint32_t i = 0; i < colorMapSize; ++i) {
		colorMap_[i] = bin.Reverse<Vector4>();
	}
}

void FinScene::FadeProcess() {
	fade_.t = std::clamp(timer_ / fadeTime_, 0.0f, 1.0f);
	backGround_.CopyBuffer(PostEffectJob::Fade, fade_);
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
		config.scale = scale_;
		config.color = colorMap_[i];
		sideBoxes_[i].SetConfig(config);
	}
}

void FinScene::UIUpdate(float deltaTime, Vector2 mousePos, std::unordered_map<Key, bool> keys) {
	bool isMouseMove = (mousePos.x != prevMousePos_.x || mousePos.y != prevMousePos_.y);
	prevMousePos_ = mousePos;
	FinSceneUI selecting = currentBox_;

	if (isMouseMove) {
		selecting = FinSceneUI::None;

		for (uint32_t i = 0; i < (uint32_t)sideBoxes_.size(); ++i) {
			if (sideBoxes_[i].IsSelected()) {
				selecting = static_cast<FinSceneUI>(i);
				break;
			}
		}

	} else {
		int tmp = int(currentBox_);

		if (keys[Key::Up]) {
			selecting = FinSceneUI((tmp - 1 + int(FinSceneUI::Count)) % int(FinSceneUI::Count));
		} else if(keys[Key::Down]) {
			selecting = FinSceneUI((tmp + 1) % int(FinSceneUI::Count));
		}

	}

	for (int i = 0; i < sideBoxes_.size(); ++i) {
		sideBoxes_[i].IsSelected(i == int(selecting));
	}

	for (uint32_t i = 0; i < sideBoxes_.size(); ++i) {
		if (sideBoxes_[i].Update(deltaTime, &orthoCamera_, mousePos)) {
			selecting = static_cast<FinSceneUI>(i);
		}
	}

	if (timer_ > fadeTime_ + setupTime_) {
		currentBox_ = selecting;
	}
}
