#include "ParticleScene.h"

ParticleScene::~ParticleScene() {
}

void ParticleScene::Initialize() {
	camera_ = std::make_unique<DebugCamera>();
	camera_->Initialize(input_);

}

std::unique_ptr<IScene> ParticleScene::Update() {
	camera_->Update();

	// 
	// 更新処理
	//

	auto fence = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(fence);

	return std::unique_ptr<IScene>();
}

void ParticleScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();
	
	directContext_->SetRenderTarget(display);

	auto fence = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fence);

	//
	// その他描画処理
	//

	display->ToTexture(directContext_);

	display->DrawImGui();
	imguiManager_.DrawImGui();

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

void ParticleScene::Save() const {
}

void ParticleScene::Load() {
}
