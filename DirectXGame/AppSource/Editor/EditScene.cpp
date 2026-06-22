#include "EditScene.h"

void EditScene::Initialize() {
	debugCamera_.Initialize(input_);
	decoEditor_ = std::make_unique<DecoEditor>(engine_, commonData_->display.get());
	grid_.Initialize(drawDataManager_);
}

std::unique_ptr<IScene> EditScene::Update() {
	debugCamera_.Update();
	grid_.Update(debugCamera_.GetCenter(), debugCamera_.GetVPMatrix());
	decoEditor_->Update(&debugCamera_, directContext_);

	return std::unique_ptr<IScene>();
}

void EditScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display, true);

	decoEditor_->Draw(directContext_);

	grid_.Draw(directContext_->GetCurrentCmdObj());

	display->ToTexture(directContext_->GetCurrentCmdObj());

#ifdef USE_IMGUI

	ImGui::Begin("FPS");
	ImGui::Text("FPS: %.1f", 1.0f / engine_->GetDeltaTime());
	ImGui::End();

#endif

	directContext_->SetRenderTarget(window, true);

	engine_->DrawImGui(directContext_->GetCurrentCmdObj());
	window->ToPresent(directContext_->GetCurrentCmdObj());
}
