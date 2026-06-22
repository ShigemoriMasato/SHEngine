#include "DecoEditor.h"

DecoEditor::DecoEditor(SHEngine::Engine* engine, SHEngine::Screen::Display* mainDisplay) {
	display_ = mainDisplay;
	decoPathManager_ = std::make_unique<Decorate::PathManager>();
	decoDataManager_ = std::make_unique<Decorate::DataManager>();
	decoObjectManager_ = std::make_unique<Decorate::ObjManager>(mainDisplay, engine, decoDataManager_.get());
	decoObjectController_ = std::make_unique<Decorate::ObjController>(mainDisplay, engine, decoDataManager_.get());
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

	decoObjectManager_->Update(camera);
	decoObjectController_->Update(camera, dcc);
}

void DecoEditor::Draw(DCC* dcc) {
	decoObjectManager_->Draw(dcc);
}
