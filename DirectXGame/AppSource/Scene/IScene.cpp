#include "IScene.h"
#include <Render/Buffer/ResourceRegistry.h>
#include <Compute/Graph/ComputeGraph.h>

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

	// ComputeShaderの実行順・割り当て (ComputeGraph ウィンドウ)。選択モードを開始するのでResourceRegistryより先に呼ぶ
	SHEngine::ComputeGraphRegistry::DrawImGui();
	// GPUBuffer・テクスチャの一覧 (Resource / Value ウィンドウ)
	SHEngine::ResourceRegistry::DrawImGui();
#endif

#ifndef USE_IMGUI
	if (initialized_) {
		pec_.Begin(display);
		pec_.End(window);
	}
	directContext_->SetRenderTarget(window, false);
#else
	directContext_->SetRenderTarget(window);
#endif

	engine_->DrawImGui();
	window->ToPresent(directContext_);
}
