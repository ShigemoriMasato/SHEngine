#include "Title.h"
#include <imgui/imgui.h>
#include <Utility/Color.h>

using namespace SHEngine;

Title::~Title() {
	Save();
}

void Title::Initialize(DrawData& drawData, Camera* camera) {
	camera_ = camera;

	isLighting_ = false;
	binaryManager_ = std::make_unique<BinaryManager>();

	Load();
}

void Title::Update(float deltaTime) {
	time_ += deltaTime;
	if (time_ > toggleTime_) {
		isLighting_ = !isLighting_;
		time_ = 0.0f;
		toggleTime_ = float(rand() % 1000) / 200.0f + 0.3f;
	}

	data.world = Matrix::MakeScaleMatrix(scale_) * Matrix::MakeTranslationMatrix(position_);
	data.vp = camera_->GetVPMatrix();
	data.color = isLighting_ ? lightColor_ : darkColor_;
	data.outlineColor = outlineColor_;
}

void Title::Draw(DCC* cmdObj) {
}

void Title::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("TitleResource");
	Vector4 colorBuffer = ConvertColor(lightColor_);
	ImGui::ColorEdit4("Light Color", &colorBuffer.x);
	lightColor_ = ConvertColor(colorBuffer);
	colorBuffer = ConvertColor(darkColor_);
	ImGui::ColorEdit4("Dark Color", &colorBuffer.x);
	darkColor_ = ConvertColor(colorBuffer);
	colorBuffer = ConvertColor(outlineColor_);
	ImGui::ColorEdit4("Outline Color", &colorBuffer.x);
	outlineColor_ = ConvertColor(colorBuffer);

	ImGui::DragFloat3("Scale", &scale_.x, 0.1f, 0.1f);
	ImGui::DragFloat3("Position", &position_.x, 0.1f);
	ImGui::End();
#endif
}

void Title::Save() {
	binaryManager_->Register(&scale_);
	binaryManager_->Register(&position_);
	binaryManager_->Register(&lightColor_);
	binaryManager_->Register(&darkColor_);
	binaryManager_->Register(&outlineColor_);
	binaryManager_->Write("Title.sg");
}

void Title::Load() {
	if (binaryManager_->Boot("Title.sg")) {
		return;
	}

	scale_ = binaryManager_->Reverse<Vector3>();
	position_ = binaryManager_->Reverse<Vector3>();
	lightColor_ = binaryManager_->Reverse<uint32_t>();
	darkColor_ = binaryManager_->Reverse<uint32_t>();
	outlineColor_ = binaryManager_->Reverse<uint32_t>();
}
