#pragma once
#include <SHEngine.h>
#include <Utility/Vector.h>
#include <vector>

#include <Common/KeyConfig/InputDevice.h>
#include <Game/ShootingGame/Render/InstancedModelRenderer.h>
#include <Space/SpaceGame/SpaceInput.h>
#include <Space/SpaceGame/Camera/SpaceCameraController.h>
#include <Space/SpaceGame/Player/SpacePlayer.h>

class KeyManager;
class ImGuiWindowManager;

/// @brief 宇宙空間ゲーム本体。白い球(プレイヤー)を三人称カメラで操作する。
/// KeyManager・ImGuiWindowManagerは所有せず、外部から受け取る。
class SpaceGame {
public:

	/// @param keyManager 入力取得用(非所有, 更新は呼び出し側)
	/// @param imguiManager 調整ウィンドウ登録先(非所有)
	SpaceGame(SHEngine::Engine* engine, const KeyManager* keyManager, ImGuiWindowManager* imguiManager);
	~SpaceGame();

	/// @param screenSize 描画先の解像度
	void Initialize(const Vector2& screenSize);

	void Update(float deltaTime);

	void Draw();
	void DrawImGui();

	const Camera* GetCamera() const { return cameraController_.GetCamera(); }

	/// @brief 現在操作しているデバイス(キーボード／マウス or コントローラー)
	InputDevice GetActiveDevice() const;
	/// @brief 直近フレームの入力
	const SpaceInput& GetInput() const { return inputState_; }

private:

	void SaveConfigs() const;

	/// @brief KeyManager・Inputの結果をゲーム用の入力へ変換する
	SpaceInput MakeInput() const;
	/// @brief カメラのヨーを基準に、入力の移動方向をワールドXZへ変換する
	Vector2 ToWorldMove(const Vector2& move) const;

	void UpdateRenderers();
	void DrawGuideImGui();

	// ================- 外部(非所有) -================
	SHEngine::Engine* engine_ = nullptr;
	SHEngine::DirectCommandContext* directContext_ = nullptr;
	SHEngine::Input* engineInput_ = nullptr;
	const KeyManager* keyManager_ = nullptr;
	ImGuiWindowManager* imguiManager_ = nullptr;

	// ================- ゲーム -================
	SpacePlayer player_;
	SpaceCameraController cameraController_;
	SpaceInput inputState_{};

	// ================- 描画 -================
	InstancedModelRenderer playerRenderer_;
	std::vector<ModelInstance> playerInstances_;

	// 何も無い空間だと移動が分からないため、目印の小さな球を格子状に並べる(デバッグ用, 保存しない)
	InstancedModelRenderer guideRenderer_;
	std::vector<ModelInstance> guideInstances_;
	bool showGuide_ = true;
	float guideSpacing_ = 10.0f;
	float guideRadius_ = 0.08f;
	static constexpr int kGuideHalfCountXZ = 5;	// XZ方向に -5～5 の11個
	static constexpr int kGuideHalfCountY = 2;	// Y方向に -2～2 の5個
	static constexpr uint32_t kGuideMaxCount =
		(kGuideHalfCountXZ * 2 + 1) * (kGuideHalfCountXZ * 2 + 1) * (kGuideHalfCountY * 2 + 1);
};
