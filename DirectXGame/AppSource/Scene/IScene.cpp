#include "IScene.h"

void IScene::DefaultPreDraw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);
}

void IScene::DefaultPostDraw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	display->ToTexture(directContext_);

	display->DrawImGui();

#ifdef USE_IMGUI
	ImGui::Begin("System");
	float deltatime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %.2f", 1.0f / deltatime);
	ImGui::End();
#endif

	directContext_->SetRenderTarget(window);
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}
