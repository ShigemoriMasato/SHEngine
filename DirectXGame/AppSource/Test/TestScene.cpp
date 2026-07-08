#include "TestScene.h"
#include <Editor/EditScene.h>

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	grid_ = std::make_unique<Grid>();
	grid_->Initialize(drawDataManager_);

	// ===================== 超えられない壁 =================================

	meshRenderer_ = std::make_unique<SHEngine::MeshRenderer>();
	meshRenderer_->SetMS("Mesh/Sample.MS.hlsl");
	meshRenderer_->SetPS("White.PS.hlsl");

	cylinder_ = std::make_unique<Cylinder>(engine_);
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	float deltaTime = engine_->GetDeltaTime();

	if (cylinder_) {
		cylinder_->Update(deltaTime, debugCamera_->GetVPMatrix());
	}

#ifdef USE_IMGUI
	ImGui::Begin("Debug");
	if (ImGui::Button("CylinderDestroy")) {
		cylinder_ = nullptr;
	}
	if (ImGui::Button("EditScene")) {
		ImGui::End();
		return std::make_unique<EditScene>();
	}
	ImGui::End();
#endif

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(display);

	grid_->Draw(directContext_);
	//meshRenderer_->Draw(directContext_);

	if (cylinder_) {
		cylinder_->Draw(directContext_);
	}

	display->ToPresent(directContext_);


	directContext_->SetRenderTarget(window);

#ifdef USE_IMGUI

	ImGui::Begin("FPS");
	float deltaTime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %f", 1.f / deltaTime);
	ImGui::End();

#endif

	commonData_->display->DrawImGui();
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}
