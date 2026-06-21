#include "DecoEditor.h"

DecoEditor::DecoEditor(SHEngine::Engine* engine, SHEngine::Screen::Display* mainDisplay) {
	display_ = mainDisplay;
	decoPathManager_ = std::make_unique<Decorate::DecoPathManager>();
	decoObjectManager_ = std::make_unique<Decorate::ObjManager>(mainDisplay, engine);
	decoObjectController_ = std::make_unique<Decorate::ObjController>(mainDisplay, engine);
}

void DecoEditor::Update(Camera* camera, DCC* dcc) {
#ifdef USE_IMGUI

	ImGui::Begin("DecoEditor");

	decoPathManager_->DrawImGui();
	if (ImGui::Button("Update")) {
		decoPathManager_->UpdateDecoPath();
	}

	ImGui::End();

#endif // USE_IMGUI

	decoObjectController_->Update(decoObjectManager_.get(), dcc);
	decoObjectManager_->Update(camera);
}

void DecoEditor::Draw(DCC* dcc) {
	decoObjectManager_->Draw(dcc);
}
