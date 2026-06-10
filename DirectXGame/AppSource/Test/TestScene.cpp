#include "TestScene.h"

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	grid_ = std::make_unique<Grid>();
	grid_->Initialize(drawDataManager_);

	// ===================== 超えられない壁 =================================

	ddsTest_ = std::make_unique<DDSTest>();
	ddsTest_->Initialize(engine_);
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	float deltaTime = engine_->GetDeltaTime();
	ddsTest_->Update(deltaTime, debugCamera_.get());

	return std::unique_ptr<IScene>();
}

void TestScene::Draw() {
	auto cmdObj = directContext_->GetCurrentCmdObj();
	auto window = commonData_->window.get();
	auto display = commonData_->display->GetDisplay();

	cmdObj->SetRenderTarget(display);

	ddsTest_->Draw(cmdObj);

	grid_->Draw(cmdObj);

	display->ToPresent(cmdObj);


	cmdObj->SetRenderTarget(window, false);

#ifdef USE_IMGUI

	ImGui::Begin("FPS");
	float deltaTime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %f", 1.f / deltaTime);
	ImGui::End();

#endif

	commonData_->display->DrawImGui();
	engine_->DrawImGui(cmdObj);
	window->ToPresent(cmdObj);
}
