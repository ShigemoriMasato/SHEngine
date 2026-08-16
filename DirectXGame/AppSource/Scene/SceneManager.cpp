#include "SceneManager.h"
#include "InitializeScene.h"

void SceneManager::Initialize(SHEngine::Engine* engine) {
	engine_ = engine;
	commonData_ = std::make_unique<CommonData>();
	nextScene_ = std::make_unique<InitializeScene>();
	currentScene_ = nullptr;
}

void SceneManager::Update() {
	if (nextScene_) {
		//engine_->StopGPU();
		engine_->GetTextureManager()->ClearIntermediateResource();
		engine_->EditImGuiIni("imgui");
		nextScene_->Ready(engine_, commonData_.get());
		nextScene_->Initialize();
		currentScene_ = std::move(nextScene_);
	}

	ImGuizmo::OriginalSetRect(commonData_->display.get());

	if (currentScene_) {
		nextScene_ = currentScene_->Update();
	}
}

void SceneManager::Draw() {
	engine_->GetTextureManager()->UploadResources(engine_->GetDirectCommandContext()->GetCommandList());

	if (currentScene_) {
		currentScene_->Draw();
	}

	engine_->GetDirectCommandContext()->EndFrame();
	engine_->GetComputeCommandContext()->EndFrame();

	commonData_->window->Present();
}

bool SceneManager::IsLoop() const {
	if (commonData_->window && commonData_->window->IsDestoroy()) {
		return false;
	}
	return true;
}
