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
		engine_->StopGPU();
		engine_->GetTextureManager()->ClearIntermediateResource();
		nextScene_->Ready(engine_, commonData_.get());
		nextScene_->Initialize();
		currentScene_ = std::move(nextScene_);
	}

	if (currentScene_) {
		nextScene_ = currentScene_->Update();
	}
}

void SceneManager::Draw() {
	engine_->GetTextureManager()->UploadResources(engine_->GetDirectCommandContext()->GetCurrentCmdObj());

	if (currentScene_) {
		currentScene_->Draw();
	}

	engine_->GetDirectCommandContext()->EndFrame();
	commonData_->window->Present();
}

bool SceneManager::IsLoop() const {
	//CommonDataとかからループを抜ける条件を判断する
	return true;
}
