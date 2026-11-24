#include "SpaceGame.h"
#include <Common/KeyConfig/KeyManager.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <Game/ShootingGame/Common/GameMath.h>
#include <algorithm>
#include <cmath>
#include <imgui/imgui.h>

SpaceGame::SpaceGame(SHEngine::Engine* engine, const KeyManager* keyManager, ImGuiWindowManager* imguiManager)
	: engine_(engine)
	, directContext_(engine->GetDirectCommandContext())
	, engineInput_(engine->GetInput())
	, keyManager_(keyManager)
	, imguiManager_(imguiManager) {
}

SpaceGame::~SpaceGame() {
	SaveConfigs();
}

void SpaceGame::Initialize(const Vector2& screenSize) {
	player_.Initialize();
	cameraController_.Initialize(screenSize, player_.GetPosition());

	// モデルの設定(テスト用の球を白で描く)
	SHEngine::ModelManager* modelManager = engine_->GetModelManager();
	const ModelData* sphere = modelManager->GetModelData(SHEngine::TestModel::Sphere);
	playerRenderer_.Initialize(sphere, 1);
	playerInstances_.resize(1);
	guideRenderer_.Initialize(sphere, kGuideMaxCount);
	guideInstances_.reserve(kGuideMaxCount);

	// ImGuiManagerに描画関数を登録する
	imguiManager_->Add([&]() {
		cameraController_.DrawImGui();
		}, "Camera");
	imguiManager_->Add([&]() {
		player_.DrawImGui();
		}, "Player");
	imguiManager_->Add([&]() {
		DrawGuideImGui();
		}, "Guide");
}

void SpaceGame::Update(float deltaTime) {
	inputState_ = MakeInput();

	// 移動方向は「現在のカメラの向き」基準にする(前フレームのカメラで決め、その後カメラを更新する)
	player_.Update(deltaTime, ToWorldMove(inputState_.move), inputState_.vertical);
	cameraController_.Update(deltaTime, player_.GetPosition(), inputState_);

	UpdateRenderers();
}

void SpaceGame::Draw() {
	const Camera* camera = cameraController_.GetCamera();

	if (showGuide_) {
		guideRenderer_.Draw(directContext_, camera);
	}
	playerRenderer_.Draw(directContext_, camera);
}

void SpaceGame::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("SpaceStatus");
	ImGui::Text("Device  : %s", GetActiveDevice() == InputDevice::Controller ? "Controller" : "Keyboard/Mouse");
	ImGui::Text("Move    : %.2f, %.2f", inputState_.move.x, inputState_.move.y);
	ImGui::Text("Vertical: %.2f", inputState_.vertical);
	const Vector3& position = player_.GetPosition();
	ImGui::Text("Position: %.2f, %.2f, %.2f", position.x, position.y, position.z);
	ImGui::End();
#endif
}

InputDevice SpaceGame::GetActiveDevice() const {
	return keyManager_->GetActiveDevice();
}

void SpaceGame::SaveConfigs() const {
	// 調整値だけを保存する(実行中状態は保存しない)。初回はファイルが既定値で作られる
	player_.SaveConfig();
	cameraController_.SaveConfig();
}

SpaceInput SpaceGame::MakeInput() const {
	auto key = keyManager_->GetKeyStates();

	SpaceInput input;
	input.device = keyManager_->GetActiveDevice();

	// ================- 水平移動 -================
	// スティックはデッドゾーン適用後のアナログ量をそのまま使う(一律に長さ1へ正規化しない)
	const Vector2 stick = keyManager_->GetLeftStick();
	if (stick.Length() > 0.0f) {
		input.move = stick;
	} else {
		// キーボードは縦横を合成し、斜めが速くならないよう長さ1へ正規化する
		Vector2 digital = {
			(key[Key::Right] ? 1.0f : 0.0f) - (key[Key::Left] ? 1.0f : 0.0f),
			(key[Key::Up] ? 1.0f : 0.0f) - (key[Key::Down] ? 1.0f : 0.0f)
		};
		if (!GameMath::TryNormalize(digital, input.move)) {
			input.move = { 0.0f, 0.0f };
		}
	}

	// ================- 上昇・下降 -================
	input.vertical = (key[Key::Ascend] ? 1.0f : 0.0f) - (key[Key::Descend] ? 1.0f : 0.0f);

	// ================- 視点 -================
	input.lookStick = keyManager_->GetRightStick();

	bool mouseLook = true;
	if (cameraController_.GetConfig().mouseLookOnRightClick) {
		mouseLook = engineInput_->GetMouseButtonState()[1] != 0;
	}
#ifdef USE_IMGUI
	// ImGuiのウィンドウを操作している間は視点を動かさない
	if (ImGui::GetIO().WantCaptureMouse) {
		mouseLook = false;
	}
#endif
	if (mouseLook) {
		input.lookMouse = engineInput_->GetMouseMove();
	}

	return input;
}

Vector2 SpaceGame::ToWorldMove(const Vector2& move) const {
	// ヨー0で前 = +Z、右 = +X(左手系)
	const float yaw = cameraController_.GetYaw();
	const Vector2 forward = { std::sin(yaw), std::cos(yaw) };
	const Vector2 right = { std::cos(yaw), -std::sin(yaw) };
	return {
		right.x * move.x + forward.x * move.y,
		right.y * move.x + forward.y * move.y
	};
}

void SpaceGame::UpdateRenderers() {
	// プレイヤー: 描画半径 = config.radius になるようモデルの実寸からスケールする
	{
		const float scale = player_.GetRadius() / playerRenderer_.GetModelRadius();
		ModelInstance& instance = playerInstances_.front();
		instance.world = GameMath::MakeWorldMatrix(scale, Matrix4x4::Identity(), player_.GetPosition());
		instance.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		instance.param = { 0.0f, 0.0f, 0.0f, 0.0f };
		playerRenderer_.SetInstances(playerInstances_);
	}

	// 目印の格子
	guideInstances_.clear();
	if (showGuide_) {
		const float scale = guideRadius_ / guideRenderer_.GetModelRadius();
		for (int y = -kGuideHalfCountY; y <= kGuideHalfCountY; ++y) {
			for (int z = -kGuideHalfCountXZ; z <= kGuideHalfCountXZ; ++z) {
				for (int x = -kGuideHalfCountXZ; x <= kGuideHalfCountXZ; ++x) {
					ModelInstance& instance = guideInstances_.emplace_back();
					const Vector3 position = {
						static_cast<float>(x) * guideSpacing_,
						static_cast<float>(y) * guideSpacing_,
						static_cast<float>(z) * guideSpacing_
					};
					instance.world = GameMath::MakeWorldMatrix(scale, Matrix4x4::Identity(), position);
					instance.color = { 0.4f, 0.4f, 0.4f, 1.0f };
					instance.param = { 0.0f, 0.0f, 0.0f, 0.0f };
				}
			}
		}
	}
	guideRenderer_.SetInstances(guideInstances_);
}

void SpaceGame::DrawGuideImGui() {
#ifdef USE_IMGUI
	ImGui::Checkbox("Show Guide", &showGuide_);
	ImGui::DragFloat("Spacing", &guideSpacing_, 0.1f, 0.5f, 1000.0f);
	ImGui::DragFloat("Radius", &guideRadius_, 0.01f, 0.01f, 10.0f);
#endif
}
