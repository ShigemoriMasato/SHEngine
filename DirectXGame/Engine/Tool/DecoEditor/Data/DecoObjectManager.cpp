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
		dataManager_->Begin(HistoryType::Add, &dummy, sizeof(dummy));
		dataManager_->End(currentPath_.data(), currentPath_.size());
	}

	auto& objectInfos = dataManager_->GetObjectInfos(currentPath_);
	for (const auto& [path, info] : objectInfos) {
		auto& renderer = renderers_[path];

		//Rendererがないときは作成する
		if (!renderer) {
			auto ddm = engine_->GetDrawDataManager();
			auto mm = engine_->GetModelManager();
			auto modelData = mm->GetNodeModelData(mm->LoadModel(path));
			auto drawData = ddm->GetDrawData(modelData.drawDataIndex);
			int textureIndex = modelData.materials[modelData.materialIndex.front()].textureIndex;
			renderer = std::make_unique<ObjRenderer>(drawData, textureIndex);
		}

		std::vector<Matrix4x4> transforms;
		std::vector<uint32_t> ids;
		transforms.reserve(info.size());
		ids.reserve(info.size());
		for (const auto& [id, transform] : info) {
			transforms.push_back(transform.Matrix());
			ids.push_back(id);
		}
		renderer->SetObjInfo(transforms, ids);
		renderer->Update(camera);
	}
}

void Decorate::ObjManager::Draw(DCC* dcc) {
	for (const auto& [path, renderer] : renderers_) {
		renderer->Draw(dcc);
	}
}
