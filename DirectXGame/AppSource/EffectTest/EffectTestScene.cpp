#include "EffectTestScene.h"

void EffectTestScene::Initialize() {
	camera_.Initialize(input_);

	effect_ = std::make_unique<Effect>();
	effect_->Initialize(engine_);

	hitEffect_ = std::make_unique<HitEffect>();
	hitEffect_->Initialize(engine_);

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex), true);
	peConfig_.origin = commonData_->display.get();
	peConfig_.output = commonData_->window.get();
	peConfig_.jobs_ = uint32_t(PostEffectJob::None);
	peConfig_.dcc = directContext_;

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);
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

	directContext_->BeginTimeStamp("Particle Draw");
	effect_->Draw();
	directContext_->EndTimeStamp();

	directContext_->SetRenderTarget(display, false);

	hitEffect_->Draw(directContext_);

	timeViewer_->Add("Particle Update", engine_->GetComputeCommandContext()->GetTimeStampResult("Particle Update"));
	timeViewer_->Add("Particle Draw", directContext_->GetTimeStampResult("Particle Draw"));
	timeViewer_->Add("DeltaTime", engine_->GetDeltaTime());
	timeViewer_->Draw(directContext_);

	display->ToTexture(directContext_);

	display->DrawImGui();
	timeViewer_->DrawImGui();

	bool isFill = true;
#ifdef SH_RELEASE

	postEffect_->Draw(peConfig_);
	isFill = false;

#endif

	directContext_->SetRenderTarget(window, isFill);


	engine_->DrawImGui();
	window->ToPresent(directContext_);
}
