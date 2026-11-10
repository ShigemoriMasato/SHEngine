#pragma once
#include <SHEngine.h>
#include "CommonData.h"
#include <Render/PostEffect/PostEffectContext.h>

class IScene {
public:

	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual std::unique_ptr<IScene> Update() = 0;
	virtual void Draw() = 0;

	void Ready(SHEngine::Engine* engine, CommonData* commonData) {
		textureManager_ = engine->GetTextureManager();
		fontLoader_ = engine->GetFontLoader();
		modelManager_ = engine->GetModelManager();
		directContext_ = engine->GetDirectCommandContext();
		computeContext_ = engine->GetComputeCommandContext();
		input_ = engine->GetInput();
		engine_ = engine;
		commonData_ = commonData;

		if (commonData_->display) {
			initialized_ = true;
			auto size = commonData_->display->GetTextureData()->GetSize();
			pec_.Initialize((uint32_t)size.first, (uint32_t)size.second);
		}
	}

protected:

	void DefaultPreDraw();
	void DefaultPostDraw();

	CommonData* commonData_ = nullptr;

	SHEngine::TextureManager* textureManager_ = nullptr;
	SHEngine::FontLoader* fontLoader_ = nullptr;
	SHEngine::ModelManager* modelManager_ = nullptr;
	SHEngine::DirectCommandContext* directContext_ = nullptr;
	SHEngine::ComputeCommandContext* computeContext_ = nullptr;

	SHEngine::Input* input_ = nullptr;

	SHEngine::Engine* engine_ = nullptr;

private:

	bool initialized_ = false;
	PostEffectContext pec_;

};
