#include "DecoEditor.h"

DecoEditor::DecoEditor(SHEngine::Engine* engine, SHEngine::Screen::Display* mainDisplay) {
	display_ = mainDisplay;
	decoDataManager_ = std::make_unique<Decorate::DataManager>();
	decoPathManager_ = std::make_unique<Decorate::PathManager>(engine->GetTextureManager(), decoDataManager_.get());
	decoObjectManager_ = std::make_unique<Decorate::ObjManager>(mainDisplay, engine, decoDataManager_.get());
	decoObjectController_ = std::make_unique<Decorate::ObjController>(mainDisplay, engine, decoDataManager_.get());
}

DecoEditor::~DecoEditor() {
}

void DecoEditor::Update(Camera* camera, DCC* dcc) {
	decoDataManager_->SetCamera(camera);

#ifdef USE_IMGUI

	ImGui::Begin("DecoEditor");

	decoPathManager_->DrawImGui();
	if (ImGui::Button("Update")) {
		decoPathManager_->UpdateDecoPath();
	}

	ImGui::End();

	decoDataManager_->DrawImGui();

#endif // USE_IMGUI

	decoObjectManager_->Update(camera);
	decoObjectController_->Update(camera, dcc);
}

void DecoEditor::Draw(DCC* dcc) {
	decoObjectManager_->Draw(dcc);
}

void DecoEditor::NormalDraw(DCC* dcc) {
	decoObjectManager_->NormalDraw(dcc);
}

void DecoEditor::SetData(const DecoObjData& data) {
	decoDataManager_->SetObjectInfos(data);
}

void DecoEditor::SetDrawCamera(Camera* camera) {
	decoObjectManager_->SetCamera(camera);
}

void DecoEditor::GetCurrentObj(std::string& path, uint32_t& id) {
	path = decoDataManager_->GetCurrentPath();
	id = decoDataManager_->GetCurrentID();
}

const DecoObjData& DecoEditor::GetData() const {
	return decoDataManager_->GetObjectInfos();
}
