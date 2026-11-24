#pragma once
#include <Scene/IScene.h>
#include <Tool/ImGui/ImGuiWindowManager.h>

#include <Space/SpaceGame/SpaceGame.h>

/// @brief 宇宙空間ゲームのシーン。入力更新・ImGuiなどの共通処理を行い、ゲーム本体はSpaceGameへ委譲する。
class SpaceScene : public IScene {
public:

	~SpaceScene() override;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	// 1フレームのdeltaTimeの上限(秒)。ヒッチ時の大移動を防ぐ
	static constexpr float kMaxDeltaTime = 1.0f / 20.0f;

	ImGuiWindowManager imguiManager_;

	// ゲーム本体。ImGuiWindowManagerを非所有で参照するため、それより後に宣言する(先に破棄される)
	std::unique_ptr<SpaceGame> spaceGame_ = nullptr;
};
