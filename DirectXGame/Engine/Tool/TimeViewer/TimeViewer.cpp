#include "TimeViewer.h"
#include <Utility/ConvertString.h>

void TimeViewer::Initialize(SHEngine::Engine* engine) {
	auto mm = engine->GetModelManager();
	planeMesh_ = mm->GetModelData(SHEngine::TestModel::Plane)->meshes.front();

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	Load();
}

void TimeViewer::Add(std::string name, double time) {
	const auto& it = texts_.find(name);
	//すでにあるならテキストを更新して終わり
	if (it != texts_.end()) {
		it->second->SetText(std::format(L"{}: {:.2f} ms", ConvertString(name), time * 1000.0));
		return;
	}

	//なければ新しく作る
	auto& text = texts_[name];
	text = std::make_unique<SHEngine::Text>();
	text->Initialize(planeMesh_, "Assets/Fonts/YDWbananaslipplus.otf", 64, name);
	text->SetText(std::format(L"{}: {:.2f} ms", ConvertString(name), time * 1000.0));
}

void TimeViewer::Draw(DCC* direct) {
	//座標の更新をする
	float offsetY = 0;
	Transform transform;
	transform.scale = { scale_, -scale_, 1.0f };
	for (auto& [name, text] : texts_) {
		transform.position = { offset_.x, offset_.y + offsetY, 0.0f };
		offsetY += interval_;
		text->SetTransform(transform);
		text->SetColor(color_);

		text->Update(orthoCamera_.GetVPMatrix());
		text->Draw(direct);
	}
}

void TimeViewer::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("TimeViewer");

	ImGui::DragFloat("Scale", &scale_, 0.01f);
	ImGui::DragFloat("Interval", &interval_, 1.0f);
	ImGui::DragFloat2("Offset", &offset_.x, 0.1f);
	ImGui::ColorEdit4("Color", &color_.x);

	ImGui::End();

#endif // USE_IMGUI

}

void TimeViewer::Save() {
	BinaryManager binManager;
	const std::string fileName = "TimeViewerConfig.bin";

	binManager.Register(&scale_);
	binManager.Register(&interval_);
	binManager.Register(&offset_);
	binManager.Register(&color_);
	binManager.Write(fileName);
}

void TimeViewer::Load() {
	BinaryManager binManager;
	const std::string fileName = "TimeViewerConfig.bin";

	if (!binManager.Boot(fileName)) {
		return;
	}

	scale_ = binManager.Reverse<float>();
	interval_ = binManager.Reverse<float>();
	offset_ = binManager.Reverse<Vector2>();
	color_ = binManager.Reverse<Vector4>();
}
