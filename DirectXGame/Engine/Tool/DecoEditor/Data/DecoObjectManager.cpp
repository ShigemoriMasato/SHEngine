#include "DecoObjectManager.h"

Decorate::ObjManager::ObjManager(SHEngine::Screen::Display* display, SHEngine::Engine* engine, DataManager* dataManager) {
	display_ = display;
	engine_ = engine;
	dataManager_ = dataManager;
}

void Decorate::ObjManager::Update(Camera* camera) {
#ifdef USE_IMGUI
	const ImGuiPayload* payload = display_->DrawImGuiWithDD("DECO_DROP");

	if (payload && payload->IsDelivery()) {
		currentPath_ = static_cast<const char*>(payload->Data);
	} else {
		currentPath_ = "";
	}

#endif

	//Dragされたので、DecoObjectを追加する
	if (!currentPath_.empty()) {
		constexpr static float cameraDistance = 20.0f;
		Vector3 position = camera->GetPosition() + camera->GetDirection() * cameraDistance;
		Vector3 dummy = position;
		dummy.x -= 1.0f;
		dataManager_->AddObject(currentPath_, position);
	}

	for (auto& [path, renderer] : renderers_) {
		renderer->Update(camera);
	}

	auto& objectInfos = dataManager_->GetObjectInfos(currentPath_);
	for (const auto& [path, info] : objectInfos) {
		auto& renderer = renderers_[path];

		//Rendererがないときは作成する
		if (!renderer) {
			auto mm = engine_->GetModelManager();
			auto modelData = mm->LoadModel(path);

			renderer = std::make_unique<ModelDrawer>();
			renderer->Initialize(modelData, path, ModelDrawer::Type::Deco);
		}

		std::vector<Matrix4x4> transforms;
		std::vector<uint32_t> ids;
		transforms.reserve(info.size());
		ids.reserve(info.size());
		for (const auto& [id, transform] : info) {
			transforms.push_back(transform.Matrix());
			ids.push_back(id);
		}
		renderer->SetTransform(transforms);
		renderer->SetIDs(ids);
	}
}

void Decorate::ObjManager::Draw(DCC* dcc) {
	for (const auto& [path, renderer] : renderers_) {
		renderer->Draw(dcc);
	}
}
