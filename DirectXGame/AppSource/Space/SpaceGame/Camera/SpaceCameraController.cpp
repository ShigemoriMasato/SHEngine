#include "SpaceCameraController.h"
#include <Space/SpaceGame/SpaceInput.h>
#include <Game/ShootingGame/Common/ConfigFile.h>
#include <Game/ShootingGame/Common/GameMath.h>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <imgui/imgui.h>

void SpaceCameraConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(distance);
	manager.Register(mouseSensitivity);
	manager.Register(stickSensitivity);
	manager.Register(minPitch);
	manager.Register(maxPitch);
	manager.Register(followSharpness);
	manager.Register(fovY);
	manager.Register(nearClip);
	manager.Register(farClip);
	manager.Register(invertY);
	manager.Register(mouseLookOnRightClick);
}

bool SpaceCameraConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(distance);
	manager.Reverse(mouseSensitivity);
	manager.Reverse(stickSensitivity);
	manager.Reverse(minPitch);
	manager.Reverse(maxPitch);
	manager.Reverse(followSharpness);
	manager.Reverse(fovY);
	manager.Reverse(nearClip);
	manager.Reverse(farClip);
	manager.Reverse(invertY);
	manager.Reverse(mouseLookOnRightClick);
	return true;
}

void SpaceCameraConfig::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragFloat("Distance", &distance, 0.1f, 1.0f, 500.0f);
	ImGui::DragFloat("Mouse Sensitivity", &mouseSensitivity, 0.0001f, 0.0f, 0.1f, "%.4f");
	ImGui::DragFloat("Stick Sensitivity", &stickSensitivity, 0.01f, 0.0f, 20.0f);
	ImGui::DragFloat("Min Pitch", &minPitch, 0.01f, -1.55f, 0.0f);
	ImGui::DragFloat("Max Pitch", &maxPitch, 0.01f, 0.0f, 1.55f);
	ImGui::DragFloat("Follow Sharpness", &followSharpness, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("FovY", &fovY, 0.01f, 0.1f, 2.5f);
	ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("Far Clip", &farClip, 1.0f, 10.0f, 100000.0f);
	ImGui::Checkbox("Invert Y", &invertY);
	ImGui::Checkbox("Mouse Look On Right Click", &mouseLookOnRightClick);
#endif
}

void SpaceCameraConfig::Validate() {
	const SpaceCameraConfig defaults{};
	distance = ConfigFile::Sanitize(distance, 1.0f, 500.0f, defaults.distance);
	mouseSensitivity = ConfigFile::Sanitize(mouseSensitivity, 0.0f, 0.1f, defaults.mouseSensitivity);
	stickSensitivity = ConfigFile::Sanitize(stickSensitivity, 0.0f, 20.0f, defaults.stickSensitivity);
	minPitch = ConfigFile::Sanitize(minPitch, -1.55f, 0.0f, defaults.minPitch);
	maxPitch = ConfigFile::Sanitize(maxPitch, 0.0f, 1.55f, defaults.maxPitch);
	followSharpness = ConfigFile::Sanitize(followSharpness, 0.1f, 100.0f, defaults.followSharpness);
	fovY = ConfigFile::Sanitize(fovY, 0.1f, 2.5f, defaults.fovY);
	nearClip = ConfigFile::Sanitize(nearClip, 0.01f, 10.0f, defaults.nearClip);
	farClip = ConfigFile::Sanitize(farClip, 10.0f, 100000.0f, defaults.farClip);
	farClip = std::max(farClip, nearClip + 1.0f);
}

void SpaceCameraController::Initialize(const Vector2& screenSize, const Vector3& target) {
	ConfigFile::Load(SpaceCameraConfig::kFileName, config_);

	screenSize_ = { std::max(screenSize.x, 1.0f), std::max(screenSize.y, 1.0f) };
	focus_ = target;
	yaw_ = 0.0f;
	pitch_ = std::clamp(0.3f, config_.minPitch, config_.maxPitch);

	ApplyProjection();
	ApplyMatrix();
}

void SpaceCameraController::Update(float deltaTime, const Vector3& target, const SpaceInput& input) {
	// ImGuiで範囲外の値が入っても破綻しないよう、毎フレーム検証する
	config_.Validate();

	// 視点回転: マウスは移動量(ピクセル)、スティックは傾き×時間
	const float ySign = config_.invertY ? -1.0f : 1.0f;
	const float yawDelta = input.lookMouse.x * config_.mouseSensitivity
		+ input.lookStick.x * config_.stickSensitivity * deltaTime;
	// マウスは下移動(+Y)で見下ろし、スティックは上倒し(+Y)で見上げ
	const float pitchDelta = (input.lookMouse.y * config_.mouseSensitivity
		- input.lookStick.y * config_.stickSensitivity * deltaTime) * ySign;

	yaw_ += yawDelta;
	// 長時間回し続けても精度が落ちないよう [-π, π] に収める
	constexpr float kPi = std::numbers::pi_v<float>;
	yaw_ = std::remainder(yaw_, 2.0f * kPi);
	pitch_ = std::clamp(pitch_ + pitchDelta, config_.minPitch, config_.maxPitch);

	// フレームレートに依存しない指数補間で注視点を追従させる
	const float t = 1.0f - std::exp(-config_.followSharpness * deltaTime);
	focus_ = focus_ + (target - focus_) * t;

	ApplyProjection();
	ApplyMatrix();
}

void SpaceCameraController::DrawImGui() {
#ifdef USE_IMGUI
	config_.DrawImGui();

	ImGui::Separator();
	ImGui::Text("Yaw  : %.3f", yaw_);
	ImGui::Text("Pitch: %.3f", pitch_);
	if (ImGui::Button("Reset Rotation")) {
		yaw_ = 0.0f;
		pitch_ = std::clamp(0.3f, config_.minPitch, config_.maxPitch);
	}
#endif
}

void SpaceCameraController::SaveConfig() const {
	ConfigFile::Save(SpaceCameraConfig::kFileName, config_);
}

void SpaceCameraController::ApplyProjection() {
	PerspectiveFovDesc desc;
	desc.SetValue(screenSize_.x, screenSize_.y, config_.fovY, config_.nearClip, config_.farClip);
	camera_.SetProjectionMatrix(desc);
}

void SpaceCameraController::ApplyMatrix() {
	// ヨー・ピッチからカメラの前方向を求め、注視点から後方へ distance 離れた位置に置く
	const float cosPitch = std::cos(pitch_);
	const Vector3 forward = {
		std::sin(yaw_) * cosPitch,
		-std::sin(pitch_),
		std::cos(yaw_) * cosPitch
	};
	const Vector3 eye = focus_ - forward * config_.distance;

	// 左手系・行ベクトル規約のLookAt(ピッチは±90°未満に制限しているので、ワールド上方向との外積は潰れない)
	Vector3 right = { 1.0f, 0.0f, 0.0f };
	GameMath::TryNormalize(GameMath::Cross({ 0.0f, 1.0f, 0.0f }, forward), right);
	const Vector3 up = GameMath::Cross(forward, right);

	const Matrix4x4 view = {
		right.x, up.x, forward.x, 0.0f,
		right.y, up.y, forward.y, 0.0f,
		right.z, up.z, forward.z, 0.0f,
		-GameMath::Dot(right, eye), -GameMath::Dot(up, eye), -GameMath::Dot(forward, eye), 1.0f
	};

	camera_.SetTransform(view);
	camera_.MakeMatrix();
}
