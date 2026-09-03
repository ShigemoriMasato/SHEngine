#include "DecoObjectManager.h"

Decorate::ObjManager::ObjManager(SHEngine::Screen::Display* display, SHEngine::Engine* engine, DataManager* dataManager) {
	display_ = display;
	engine_ = engine;
	dataManager_ = dataManager;
}

void Decorate::ObjManager::Update(Camera* camera) {
	//Dragされたので、DecoObjectを追加する
	if (!currentPath_.empty()) {
		dataManager_->AddObject(currentPath_, Vector3(0, 0, 0));
	}

	for (auto& [path, renderer] : renderers_) {
		renderer->Update(camera);
	}

	auto& objectInfos = dataManager_->GetObjectInfos();
	std::unordered_set<std::string> nonUpdate;
	for (const auto& [path, renderer] : renderers_) {
		nonUpdate.insert(path);
	}

	for (const auto& [path, info] : objectInfos) {
		auto& renderer = renderers_[path];
		nonUpdate.erase(path);

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
			transforms.push_back(transform.Matrix() * parentMatrix_);
			ids.push_back(id);
		}
		renderer->SetTransform(transforms);
		renderer->SetIDs(ids);
	}

	for (const auto& path : nonUpdate) {
		renderers_[path]->SetTransform({});
		renderers_[path]->Update(camera);
	}
}

void Decorate::ObjManager::Draw(DCC* dcc) {
	for (const auto& [path, renderer] : renderers_) {
		renderer->Draw(dcc);
	}
}

void Decorate::ObjManager::NormalDraw(DCC* dcc) {
	for (const auto& [path, renderer] : renderers_) {
		renderer->NormalDraw(dcc);
	}
}

void Decorate::ObjManager::SetCamera(Camera* camera) {
	for (auto& renderer : renderers_) {
		renderer.second->Update(camera);
	}
}
