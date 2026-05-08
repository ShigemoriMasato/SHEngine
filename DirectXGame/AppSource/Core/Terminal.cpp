#include "Terminal.h"

Terminal::Terminal(HINSTANCE hInstance) {
	engine_ = std::make_unique<SHEngine::Engine>();
	engine_->Initialize(hInstance);
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(engine_.get());
}

void Terminal::Run() {

	while (IsLoop()) {
		engine_->BeginFrame();

		sceneManager_->Update();

		sceneManager_->Draw();

		engine_->PostDraw();

	}

	engine_->StopGPU(SHEngine::Command::Type::Direct);
	engine_->StopGPU(SHEngine::Command::Type::Compute);
}

bool Terminal::IsLoop() const {
	return engine_->IsLoop() && sceneManager_->IsLoop();
}
