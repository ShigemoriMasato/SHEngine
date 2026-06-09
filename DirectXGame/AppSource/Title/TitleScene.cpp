#include "TitleScene.h"
#include <Game/GameScene.h>

TitleScene::TitleScene() {
	waterWave_ = std::make_unique<WaterWave>();
	camera_ = std::make_unique<Camera>();
	title_ = std::make_unique<Title>();
}

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {
	int modelIndex = modelManager_->LoadModel("WaterPlane");
	auto model = modelManager_->GetNodeModelData(modelIndex);
	auto drawData = drawDataManager_->GetDrawData(model.drawDataIndex);
	waterWave_->Initialize(drawData, camera_.get());

	modelIndex = modelManager_->LoadModel("Title");
	model = modelManager_->GetNodeModelData(modelIndex);
	drawData = drawDataManager_->GetDrawData(model.drawDataIndex);
	title_->Initialize(drawData, camera_.get());

	auto pedd = drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex);
	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, pedd, true);		//描画だけする
	postEffectConfig_.cmdObj = commonData_->cmdObject.get();
	postEffectConfig_.origin = commonData_->display->GetDisplay();
	postEffectConfig_.jobs_ = uint32_t(PostEffectJob::None);

	camera_->SetProjectionMatrix(PerspectiveFovDesc());
	camera_->SetPosition({ 0.0f, 0.0f, -14.0f });
	camera_->MakeMatrix();
}

std::unique_ptr<IScene> TitleScene::Update() {
	commonData_->cmdObject->ResetCommandList();

	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	input_->Update();
	commonData_->keyManager->Update();

	camera_->MakeMatrix();

	waterWave_->Update(deltaTime);
	title_->Update(deltaTime);

	auto key = commonData_->keyManager->GetKeyStates();

	if (key.at(Key::Hold) || key.at(Key::HardDrop)) {
		return std::make_unique<GameScene>();
	}

	return nullptr;
}

void TitleScene::Draw() {
	auto cmdObj = commonData_->cmdObject.get();
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	display->PreDraw(cmdObj);
	
	waterWave_->Draw(cmdObj);
	title_->Draw(cmdObj);

	display->PostDraw(cmdObj);

#ifdef SH_RELEASE

	postEffect_->Draw(postEffectConfig_);
	cmdObj->SetRenderTarget(window->GetCurrentDisplay(), false);

#else

	cmdObj->SetRenderTarget(window);

#endif

	//ここ以外で記述する場合、ifdefを忘れないようにすること
#ifdef USE_IMGUI
	display->DrawImGui();
	title_->DrawImGui();
	waterWave_->DrawImGui();
	camera_->DrawImGui();
#endif

	engine_->DrawImGui();
	window->ToPresent(cmdObj);

}
