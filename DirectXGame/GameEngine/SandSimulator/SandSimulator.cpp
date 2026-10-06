#include "SandSimulator.h"
#include <imgui/imgui.h>

using namespace Sand;

SandSimulator::SandSimulator(const Sand::InitializeData& initData) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	particleData_.Initialize(container_.get(), initData.particleCount);
	gridData_.Initialize(container_.get(), initData.gridVerticalCount, initData.gridHorizontalCount, initData.gridDepthCount);
	freeList_.Initialize(container_.get(), initData.particleCount);

	processor_ = std::make_unique<Processor>();
	renderer_ = std::make_unique<Renderer>(&particleData_, initData.sandModel);
	renderer_->SetInstance(initData.particleCount);
}

void SandSimulator::Initialize(SHEngine::ICommandContext* commandContext) {
	processor_->Initialize(&particleData_, &gridData_, &freeList_, commandContext);
}

void SandSimulator::Update(float deltaTime, SHEngine::ICommandContext* commandContext) {
	processor_->Update(deltaTime, commandContext);
}

void SandSimulator::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void SandSimulator::DebugDraw(DCC* dcc) {
}

void SandSimulator::DrawImGui() {
#ifdef USE_IMGUI
	
	ImGui::Begin("Sand Simulator");
	ImGui::DragFloat("Scale: %f", &scale, 0.1f, 0.0f, 100.0f);
	ImGui::ColorEdit4("Color", (float*)&color);
	ImGui::End();

#endif

	particleData_.scale->CopyBuffer(&scale, sizeof(float));
	particleData_.color->CopyBuffer(&color, sizeof(Vector4));
}

void SandSimulator::SetCamera(const Camera* camera) {
	renderer_->SetCamera(camera);
}
