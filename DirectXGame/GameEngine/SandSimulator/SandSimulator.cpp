#include "SandSimulator.h"
#include <imgui/imgui.h>

using namespace Sand;

SandSimulator::SandSimulator(const Sand::InitializeData& initData) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	particleData_.Initialize(container_.get(), initData.particleCount);
	gridData_.Initialize(container_.get(), initData.particleCount);

	configure_ = std::make_unique<Configure>();

	processor_ = std::make_unique<Processor>(container_.get(), &particleData_, &gridData_, configure_.get());
	renderer_ = std::make_unique<Renderer>(&particleData_, initData.sandModel);
	renderer_->SetInstance(initData.particleCount);

#ifdef USE_IMGUI
	configure_->Add([&]() {
		ImGui::DragFloat3("position", (float*)&spherePos_, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat("radius", &sphereRadius_, 0.01f, 0.0f, 100.0f);
		isInitialize_ = ImGui::Button("Initialize");
		ImGui::Text("Initialize: %s", isInitialize_ ? "true" : "false");
		}, "Initialize");

	configure_->Add([&]() {
		ImGui::ColorEdit4("Color", &color.x);
		ImGui::DragFloat("Scale", &scale, 0.01f);
		}, "Drawer");
#endif

	timeViewer_ = std::make_unique<TimeViewer>();
}

void SandSimulator::Initialize(SHEngine::ICommandContext* commandContext) {
	processor_->Initialize(commandContext, spherePos_, sphereRadius_);
}

void SandSimulator::Update(float deltaTime, SHEngine::ICommandContext* commandContext) {
	int processCount = processor_->GetProcessCount();
	particleData_.SetProcessNum(processCount);
	renderer_->SetInstance(processCount);

	gridData_.Update();

	processor_->Update(commandContext, deltaTime);

	particleData_.color->CopyBuffer(&color, sizeof(Vector4));
	particleData_.scale->CopyBuffer(&scale, sizeof(float));

	if (isInitialize_) {
		processor_->Initialize(commandContext, spherePos_, sphereRadius_);
	}
}

void SandSimulator::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void SandSimulator::DebugDraw(DCC* dcc) {
}

void SandSimulator::DrawImGui() {
	configure_->DrawImGui();
}

void SandSimulator::SetCamera(const Camera* camera) {
	renderer_->SetCamera(camera);
}

void SandSimulator::Save(BinaryManager* bin) const {
	bin->Register(spherePos_);
	bin->Register(sphereRadius_);

	bin->Register(color);
	bin->Register(scale);

	processor_->Save(bin);
}

void SandSimulator::Load(BinaryManager* bin) {
	bin->Reverse(spherePos_);
	bin->Reverse(sphereRadius_);

	bin->Reverse(color);
	bin->Reverse(scale);

	processor_->Load(bin);
}
