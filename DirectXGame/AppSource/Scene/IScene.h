#pragma once
#include <SHEngine.h>
#include "CommonData.h"

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
	}

protected:

	CommonData* commonData_ = nullptr;

	SHEngine::TextureManager* textureManager_ = nullptr;
	SHEngine::FontLoader* fontLoader_ = nullptr;
	SHEngine::ModelManager* modelManager_ = nullptr;
	SHEngine::DirectCommandContext* directContext_ = nullptr;
	SHEngine::ComputeCommandContext* computeContext_ = nullptr;

	SHEngine::Input* input_ = nullptr;

	SHEngine::Engine* engine_ = nullptr;

};
