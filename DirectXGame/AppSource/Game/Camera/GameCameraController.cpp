#include "GameCameraController.h"
#include <Game/Common/ConfigFile.h>
#include <Game/Common/GameMath.h>
#include <algorithm>
#include <cmath>

void CameraConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(distance);
	manager.Register(pitch);
	manager.Register(followSharpness);
	manager.Register(fovY);
	manager.Register(nearClip);
	manager.Register(farClip);
}

bool CameraConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(distance);
	manager.Reverse(pitch);
	manager.Reverse(followSharpness);
	manager.Reverse(fovY);
	manager.Reverse(nearClip);
	manager.Reverse(farClip);
	return true;
}

void CameraConfig::Validate() {
	const CameraConfig defaults{};
	distance = ConfigFile::Sanitize(distance, 1.0f, 500.0f, defaults.distance);
	pitch = ConfigFile::Sanitize(pitch, 0.1f, 1.5f, defaults.pitch);
	followSharpness = ConfigFile::Sanitize(followSharpness, 0.1f, 100.0f, defaults.followSharpness);
	fovY = ConfigFile::Sanitize(fovY, 0.1f, 2.5f, defaults.fovY);
	nearClip = ConfigFile::Sanitize(nearClip, 0.01f, 10.0f, defaults.nearClip);
	farClip = ConfigFile::Sanitize(farClip, 10.0f, 100000.0f, defaults.farClip);
}

void GameCameraController::Initialize(const Vector2& screenSize, const Vector3& target) {
	ConfigFile::Load(CameraConfig::kFileName, config_);

	PerspectiveFovDesc desc;
	const float height = std::max(screenSize.y, 1.0f);
	desc.SetValue(std::max(screenSize.x, 1.0f), height, config_.fovY, config_.nearClip, config_.farClip);
	camera_.SetProjectionMatrix(desc);

	focus_ = target;
	ApplyMatrix();
}

void GameCameraController::Update(float deltaTime, const Vector3& target) {
	// フレームレートに依存しない指数補間
	const float t = 1.0f - std::exp(-config_.followSharpness * deltaTime);
	focus_ = focus_ + (target - focus_) * t;
	ApplyMatrix();
}

void GameCameraController::SaveConfig() const {
	ConfigFile::Save(CameraConfig::kFileName, config_);
}

void GameCameraController::ApplyMatrix() {
	// 注視点の手前(-Z)上方から見下ろす。画面上方向がワールド+Zになる
	const Vector3 offset = {
		0.0f,
		std::sin(config_.pitch) * config_.distance,
		-std::cos(config_.pitch) * config_.distance
	};
	const Vector3 eye = focus_ + offset;

	// 左手系・行ベクトル規約のLookAt
	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	GameMath::TryNormalize(focus_ - eye, forward);
	Vector3 right = { 1.0f, 0.0f, 0.0f };
	GameMath::TryNormalize(GameMath::Cross({ 0.0f, 1.0f, 0.0f }, forward), right);
	const Vector3 up = GameMath::Cross(forward, right);

	const Matrix4x4 view = {
		right.x,						up.x,						forward.x,						0.0f,
		right.y,						up.y,						forward.y,						0.0f,
		right.z,						up.z,						forward.z,						0.0f,
		-GameMath::Dot(right, eye),		-GameMath::Dot(up, eye),	-GameMath::Dot(forward, eye),	1.0f
	};

	camera_.SetPosition(eye);
	camera_.SetTransform(view);
	camera_.MakeMatrix();
}
