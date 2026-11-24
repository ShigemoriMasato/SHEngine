#include "SpaceScene.h"
#include <algorithm>

SpaceScene::~SpaceScene() {
	// ゲーム(調整値の保存を含む)を、参照先のImGuiWindowManager等より先に破棄する
	spaceGame_.reset();
}

void SpaceScene::Initialize() {
	const auto size = commonData_->display->GetTextureData()->GetSize();
	const Vector2 screenSize = { static_cast<float>(size.first), static_cast<float>(size.second) };

	spaceGame_ = std::make_unique<SpaceGame>(engine_, commonData_->keyManager.get(), &imguiManager_);
	imguiManager_.PushGroup("Space");
	spaceGame_->Initialize(screenSize);
	imguiManager_.PopGroup();
}

std::unique_ptr<IScene> SpaceScene::Update() {
	commonData_->keyManager->Update();

	const float deltaTime = std::min(engine_->GetDeltaTime(), kMaxDeltaTime);
	spaceGame_->Update(deltaTime);

#ifdef SH_DEBUG
	auto key = commonData_->keyManager->GetKeyStates();
	if (key[Key::Escape]) {
		commonData_->shutDown_ = true;
	}
#endif

	return std::unique_ptr<IScene>();
}

void SpaceScene::Draw() {
	// 背景はDisplayのClearColor(黒)で塗りつぶす
	DefaultPreDraw();

	spaceGame_->Draw();

	spaceGame_->DrawImGui();
	imguiManager_.DrawImGui();

	DefaultPostDraw();
}
