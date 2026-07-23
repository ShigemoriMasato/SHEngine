#include "SideBox.h"
#include <Utility/Easing.h>

void SideBox::StaticInitialize(SHEngine::Engine* engine) {
	planeMesh_ = engine->GetModelManager()->GetModelData(SHEngine::TestModel::Plane)->meshes.front();
	int textureIndex = engine->GetTextureManager()->LoadTexture("UI/SideBox.png");
	backGroundTexture_ = std::make_unique<SHEngine::GPUBuffer>(engine->GetTextureManager()->GetTextureData(textureIndex));
	LoadCommon();
}

void SideBox::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("SelectBoxCommon");
	ImGui::DragFloat2("Scale", &textScaleOffset_.x, 0.1f);
	ImGui::DragFloat2("Position", &textPositionOffset_.x, 0.1f);
	ImGui::DragFloat("LerpSpeed", &lerpSpeed_, 0.1f);
	ImGui::DragFloat("PutRatio", &putRatio_, 0.01f, 0.0f, 1.0f);
	if (ImGui::Button("Save")) {
		SaveCommon();
	}
	ImGui::End();

#endif
}

void SideBox::Initialize(std::string sentence) {
	text_ = std::make_unique<SHEngine::Text>();
	text_->Initialize(planeMesh_, "851Gkktt_005.ttf", 64, "SelectBoxText");
	text_->SetText(sentence);
	text_->IsUI(true);

	container_ = std::make_unique<SHEngine::BufferContainer>(2);
	matrixBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));
	colorBuffer_ = container_->Create(BufferType::CBV, sizeof(Vector4));

	backGround_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::PostEffect, planeMesh_);
	backGround_->SetVS("Game/UI/SideBox.VS.hlsl");
	backGround_->SetPS("Game/UI/SideBox.PS.hlsl");
	backGround_->SetGPUBuffer(matrixBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	backGround_->SetGPUBuffer(colorBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
	backGround_->SetGPUBuffer(backGroundTexture_.get(), ShaderType::PIXEL_SHADER, BufferType::SRV);
	backGround_->SetDepthStencil(SHEngine::PSO::DepthStencilID::UI);
}


bool SideBox::Update(float deltaTime, Camera* orthoCamera, Vector2 mousePos) {
	bool isMouseInBox = IsMouseInBox(mousePos);

	if (isMouseInBox) {
		t_ += deltaTime * lerpSpeed_;
	} else {
		t_ -= deltaTime * lerpSpeed_;
	}

	t_ = std::clamp(t_, 0.0f, 1.0f);

	Vector2 putedPosition = lerp(config_.position, config_.position + config_.putDirection * config_.scale * putRatio_, t_);
	Vector2 textPosition = putedPosition + textPositionOffset_;

	Transform textTransform = {};
	textTransform.scale = Vector3(config_.scale.x * textScaleOffset_.x, config_.scale.y * textScaleOffset_.y, 1.0f);
	textTransform.position = Vector3(textPosition.x, textPosition.y, 0.0f);
	text_->SetTransform(textTransform);
	text_->SetColor(config_.color);
	text_->Update(orthoCamera->GetVPMatrix());

	Transform backGroundTransform = {};
	backGroundTransform.scale = Vector3(config_.scale.x, config_.scale.y, 1.0f);
	backGroundTransform.position = Vector3(putedPosition.x, putedPosition.y, 0.0f);
	Matrix4x4 wvpMatrix = backGroundTransform.Matrix() * orthoCamera->GetVPMatrix();

	matrixBuffer_->CopyBuffer(&wvpMatrix, sizeof(wvpMatrix));
	colorBuffer_->CopyBuffer(&config_.color, sizeof(Vector4));

	bool mouseInBoxTrigger = isMouseInBox && !prevMouseInBox_;
	prevMouseInBox_ = isMouseInBox;
	return mouseInBoxTrigger;
}

void SideBox::Draw(DCC* dcc) {
	backGround_->Draw(dcc);
	text_->Draw(dcc);
}

void SideBox::SetConfig(const Config& config) {
	config_ = config;
}

void SideBox::SaveCommon() {
	BinaryManager bin;
	bin.Register(&textPositionOffset_);
	bin.Register(&textScaleOffset_);
	bin.Register(&lerpSpeed_);
	bin.Register(&putRatio_);
	bin.Write(commonDataFileName_);
}

void SideBox::LoadCommon() {
	BinaryManager bin;
	if (!bin.Boot(commonDataFileName_)) {
		return;
	}
	textPositionOffset_ = bin.Reverse<Vector2>();
	textScaleOffset_ = bin.Reverse<Vector2>();
	Vector2 boxSize = bin.Reverse<Vector2>();
	lerpSpeed_ = bin.Reverse<float>();
	putRatio_ = bin.Reverse<float>();
}

bool SideBox::IsMouseInBox(Vector2 mousePos) const {
	Vector2 position = lerp(config_.position, config_.position + config_.putDirection * config_.scale * putRatio_, t_);
	float minX = position.x - config_.scale.x * 0.5f;
	float maxX = position.x + config_.scale.x * 0.5f;
	float minY = position.y + config_.scale.y * 0.5f;
	float maxY = position.y - config_.scale.y * 0.5f;

	bool inX = mousePos.x >= minX && mousePos.x <= maxX;
	bool inY = mousePos.y >= minY && mousePos.y <= maxY;

	return inX && inY;
}


void SideBox::Config::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragFloat2("Scale", &scale.x, 0.1f);
	ImGui::DragFloat2("Position", &position.x, 0.1f);
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::DragFloat2("PutDirection", &putDirection.x, 0.1f);
#endif
}

void SideBox::Config::Save(BinaryManager& bin) const {
	bin.Register(&scale);
	bin.Register(&position);
	bin.Register(&color);
	bin.Register(&putDirection);
}

void SideBox::Config::Load(BinaryManager& bin) {
	scale = bin.Reverse<Vector2>();
	position = bin.Reverse<Vector2>();
	color = bin.Reverse<Vector4>();
	putDirection = bin.Reverse<Vector2>();
}
