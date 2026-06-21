#include "EffectTestScene.h"

void EffectTestScene::Initialize() {
	camera_.Initialize(input_);
	{
		auto drawData = drawDataManager_->GetDrawData(modelManager_->GetNodeModelData(1).drawDataIndex);
		effect_ = std::make_unique<Effect>();
		effect_->Initialize(drawData, engine_);
	}

	hitEffect_ = std::make_unique<HitEffect>();
	hitEffect_->Initialize(engine_);
}

std::unique_ptr<IScene> EffectTestScene::Update() {
	camera_.Update();

	hitEffect_->Update(engine_->GetDeltaTime(), camera_.GetVPMatrix());
	effect_->Update(camera_.GetVPMatrix(), camera_.GetBillboardMatrix(), engine_->GetDeltaTime());

	return std::unique_ptr<IScene>();
}

void EffectTestScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	effect_->Draw(display);

	directContext_->SetRenderTarget(display, false);

	hitEffect_->Draw(directContext_);

	display->ToTexture(directContext_->GetCurrentCmdObj());

	display->DrawImGui();

	directContext_->SetRenderTarget(window, true);

	
	engine_->DrawImGui(directContext_->GetCurrentCmdObj());
	window->ToPresent(directContext_->GetCurrentCmdObj());
}
