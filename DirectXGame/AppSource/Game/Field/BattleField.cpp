#include "BattleField.h"

BattleField::BattleField(ParticleSystem* particleSystem, SHEngine::TextureManager* textureManager, ImGuiWindowManager* imguiManager, ICC* icc) {
	display_ = std::make_unique<SHEngine::Screen::Display>();
	display_->Initialize(textureSize_, textureSize_, "BattleField");
	display_->AddRenderTarget(textureManager, 0xffffff);

	emitter_ = particleSystem->CreateEmitter(textureSize_ * textureSize_, icc);
	container_ = std::make_unique<SHEngine::BufferContainer>();

	scale_ = container_->Create(BufferType::CBV, sizeof(float));
	textureConfig_ = container_->Create(BufferType::CBV, sizeof(uint32_t) * 2);
	texture_ = container_->Create(display_->GetTextureData());
	uvs_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector2), textureSize_ * textureSize_);

	const float scale = 0.1f;
	const uint32_t textureConfig[2] = { textureSize_, textureSize_ };
	scale_->CopyBuffer(&scale, sizeof(float));
	textureConfig_->CopyBuffer(textureConfig, sizeof(uint32_t) * 2);

#ifdef USE_IMGUI
	imguiManager->Add([this]() {
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Position", &transform_.position.x, 0.01f);
		ImGui::Image(ImTextureRef(display_->GetTextureData()->GetSRVHandle().ptr), { 64, 64 });
	}, "BattleField");
#endif

	update_ = emitter_->AddOprionalProcess("PixelTex/ApplyTex");
	update_->SetGPUBuffers(BufferType::SRV, { uvs_, texture_ });
	update_->SetExecuteNum(textureSize_ * textureSize_, 128);
	update_->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
}

void BattleField::Initialize() {
	SHEngine::ComputeObject* init = emitter_->AddTriggerProcess("PixelTex/Initialize");

	init->SetGPUBuffers(BufferType::CBV, { textureConfig_, scale_ });
	init->SetGPUBuffers(BufferType::SRV, { texture_ });
	init->SetGPUBuffers(BufferType::UAV, { uvs_ });
	init->SetExecuteNum(textureSize_ * textureSize_, 128);
	init->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
}

void BattleField::Update(float deltaTime, ICC* icc) {
	emitter_->SetParentMatrix(transform_.Matrix());
}

void BattleField::Draw(DCC* dcc) {
	auto renderTarget = dcc->GetRenderTarget();

	dcc->SetRenderTarget(display_.get(), true, true);

	display_->ToTexture(dcc);

	if (renderTarget) {
		dcc->SetRenderTarget(renderTarget, false, true);
	}
}

void BattleField::ApplyTexture(ICC* icc) {
	update_->Execute(icc);
}
