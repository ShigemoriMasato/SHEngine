#include "DecoObjectManager.h"

Decorate::ObjManager::ObjManager(SHEngine::Screen::Display* display, SHEngine::Engine* engine) {
	display_ = display;
	engine_ = engine;
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

		auto mm = engine_->GetModelManager();
		int modelIndex = mm->LoadModel(currentPath_);

		if (renderers_.size() <= modelIndex) {
			renderers_.resize(modelIndex + 1);
		}

		if (!renderers_[modelIndex]) {
			auto modelData = mm->GetNodeModelData(modelIndex);
			auto drawData = engine_->GetDrawDataManager()->GetDrawData(modelData.drawDataIndex);
			int textureIndex = modelData.materials[modelData.materialIndex.front()].textureIndex;
			renderers_[modelIndex] = std::make_unique<ObjRenderer>(drawData, textureIndex);

			renderInfos_.resize(modelIndex + 1);
		}

		//追加するDecoObjectの情報を作成
		renderInfos_[modelIndex].ids.push_back(nextID_++);
		Transform transform;
		static const float cameraDistance = 20.0f;
		transform.position = camera->GetPosition() + camera->GetDirection() * cameraDistance;
		renderInfos_[modelIndex].transforms.push_back(transform.Matrix());
	}

	for (int i = 0; i < int(renderers_.size()); ++i) {
		if (renderers_[i]) {
			renderers_[i]->SetObjInfo(renderInfos_[i].transforms, renderInfos_[i].ids);
			renderers_[i]->Update(camera);
		}
	}
}

void Decorate::ObjManager::Draw(DCC* dcc) {
	for (int i = 0; i < int(renderers_.size()); ++i) {
		if (renderers_[i]) {
			renderers_[i]->Draw(dcc);
		}
	}
}

void Decorate::ObjManager::SetTransform(uint32_t id, const Transform& transform) {
	if (id == 0) {
		return;
	}

	for (auto& renderInfo : renderInfos_) {
		for (int i = 0; i < int(renderInfo.ids.size()); ++i) {
			if (renderInfo.ids[i] == id) {
				renderInfo.transforms[i] = transform.Matrix();
				return;
			}
		}
	}
}

Matrix4x4 Decorate::ObjManager::GetTransform(uint32_t id) const {
	for (auto& renderInfo : renderInfos_) {
		for (int i = 0; i < int(renderInfo.ids.size()); ++i) {
			if (renderInfo.ids[i] == id) {
				return renderInfo.transforms[i];
			}
		}
	}

	return Matrix4x4::Identity();
}
