#include "ShootingStar.h"
#include <imgui/imgui.h>

ShootingStar::ShootingStar(SHEngine::BufferContainer* container, const Mesh& mesh, ImGuiWindowManager* imguiManager) {
	colorBuffer_ = container->Create(BufferType::CBV, sizeof(Vector4));
	scaleBuffer_ = container->Create(BufferType::CBV, sizeof(float));
	positionBuffer_ = container->Create(BufferType::SRV, sizeof(Vector2), starNum_);

#ifdef USE_IMGUI
	imguiManager->Add([this]() {
		ImGui::ColorEdit4("Color", &color_.x);
		ImGui::DragFloat("Scale", &scale_, 0.1f, 0.1f);
		ImGui::DragFloat2("Velocity", &velocity_.x, 0.1f);
		ImGui::Separator();
		ImGui::DragFloat("Frequency", &frequency_, 0.01f, 0.01f);
		ImGui::DragInt("EmitNum", (int*)&emitNum_, 1, 1);
	}, "ShootingStar");
#endif

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Position, mesh);
	renderer_->SetVS("Game/Field/ShootingStar.VS.hlsl");
	renderer_->SetPS("Game/Field/ShootingStar.PS.hlsl");
}

void ShootingStar::Initialize(uint32_t textureSize, Camera* camera) {
	textureSize_ = textureSize;

	renderer_->SetGPUBuffers({ camera->GetVPBuffer(), scaleBuffer_}, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(positionBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(colorBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
}

void ShootingStar::Update(float deltaTime) {
	timer_ += deltaTime;
	if (timer_ >= frequency_) {
		timer_ = 0.0f;
		for (uint32_t i = 0; i < emitNum_; ++i) {
			if (positions_.size() < starNum_) {
				Vector2 pos = { dist_(seed_) * float(textureSize_), dist_(seed_) * float(textureSize_) };
				positions_.push_back(pos);
			} else {
				break;
			}
		}
	}

	for (int i = 0; i < positions_.size(); ++i) {
		positions_[i] += velocity_ * deltaTime;
		//画面外に出たら削除
		if (positions_[i].x < 0.0f || positions_[i].x > float(textureSize_) || positions_[i].y < 0.0f || positions_[i].y > float(textureSize_)) {
			positions_.erase(positions_.begin() + i);
			--i;
		}
	}

	colorBuffer_->CopyBuffer(&color_, sizeof(Vector4));
	scaleBuffer_->CopyBuffer(&scale_, sizeof(float));
	positionBuffer_->CopyBuffer(positions_.data(), sizeof(Vector2) * positions_.size());
	renderer_->instanceNum_ = (uint32_t)positions_.size();
}

void ShootingStar::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void ShootingStar::Save(BinaryManager& manager) const {
	manager.Register(frequency_);
	manager.Register(emitNum_);
	manager.Register(velocity_);
	manager.Register(scale_);
	manager.Register(color_);
}

void ShootingStar::Load(BinaryManager& manager) {
	manager.Reverse(frequency_);
	manager.Reverse(emitNum_);
	manager.Reverse(velocity_);
	manager.Reverse(scale_);
	manager.Reverse(color_);
}
