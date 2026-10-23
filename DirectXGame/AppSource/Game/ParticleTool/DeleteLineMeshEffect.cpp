#include "DeleteLineMeshEffect.h"

void DeleteLineMeshEffect::Initialize(Tetris* tetris, PolygonEmitter* emitter, const Mesh& cubeMesh) {
	tetris_ = tetris;
	emitter_ = emitter;

	zeroConfig_.emitNum = 0;
	configs_.resize(4, zeroConfig_);
	for (int i = 0; i < 4; ++i) {
		configs_[i] = emitter_->AddPolygon({cubeMesh});
	}
}

void DeleteLineMeshEffect::Update(float deltaTime) {
	int deleteNum = tetris_->IsLineDeleted();
	auto lines = tetris_->FillLineIndex();
	int i = 0;
	for (i = 0; i < deleteNum; ++i) {
		auto& config = configs_[i];

		int line = lines[i];
		Vector2 fieldSize = tetris_->GetFieldSize();
		Vector3 lowerLeftPos = tetris_->GetLowerLeftPos();
		Vector2 mid = Vector2(lowerLeftPos.x + fieldSize.x / 2.0f, lowerLeftPos.y);
		Vector2 pos = Vector2(lowerLeftPos.x + fieldSize.x / 2.0f, lowerLeftPos.y + line + 0.5f);

		configs_[i].Copy(emitConfig_);

		configs_[i].transform.position = Vector3(pos.x, pos.y, lowerLeftPos.z) * parentMatrix_;
	}
	for (i; i < int(configs_.size()); ++i) {
		configs_[i].Copy(zeroConfig_);
	}

	if (debugDeleteLine_) {
		configs_[0].Copy(emitConfig_);
		configs_[0].transform.position = Vector3(0.0f, 0.0f, 0.0f) * parentMatrix_;
		debugDeleteLine_ = false;
	}

	for (auto& config : configs_) {
		emitter_->SetConfig(config);
	}
}

void DeleteLineMeshEffect::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("DeleteLineMeshEffect");

	emitConfig_.DrawImGui();

	if (ImGui::Button("Emit")) {
		debugDeleteLine_ = true;
	}

	ImGui::End();

#endif
}



void DeleteLineMeshEffect::Save(BinaryManager& bin) const {
	emitConfig_.Save(bin);
}

void DeleteLineMeshEffect::Load(BinaryManager& bin) {
	emitConfig_.Load(bin);
}
