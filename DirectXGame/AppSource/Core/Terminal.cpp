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

		sceneManager_->Present();
	}

	engine_->StopGPU(SHEngine::Command::Type::Direct, 0);
	for (int i = 0; i < 6; ++i) {
		engine_->StopGPU(SHEngine::Command::Type::Compute, i);
	}
}

bool Terminal::IsLoop() const {
	return engine_->IsLoop() && sceneManager_->IsLoop();
}
