#include "SpacePlayer.h"
#include <Game/ShootingGame/Common/ConfigFile.h>
#include <algorithm>
#include <cmath>
#include <imgui/imgui.h>

void SpacePlayerConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(horizontalSpeed);
	manager.Register(verticalSpeed);
	manager.Register(responsiveness);
	manager.Register(radius);
}

bool SpacePlayerConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(horizontalSpeed);
	manager.Reverse(verticalSpeed);
	manager.Reverse(responsiveness);
	manager.Reverse(radius);
	return true;
}

void SpacePlayerConfig::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::DragFloat("Horizontal Speed", &horizontalSpeed, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("Vertical Speed", &verticalSpeed, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat("Responsiveness", &responsiveness, 0.1f, 0.1f, 100.0f);
	ImGui::DragFloat("Radius", &radius, 0.01f, 0.05f, 100.0f);
#endif
}

void SpacePlayerConfig::Validate() {
	const SpacePlayerConfig defaults{};
	horizontalSpeed = ConfigFile::Sanitize(horizontalSpeed, 0.0f, 1000.0f, defaults.horizontalSpeed);
	verticalSpeed = ConfigFile::Sanitize(verticalSpeed, 0.0f, 1000.0f, defaults.verticalSpeed);
	responsiveness = ConfigFile::Sanitize(responsiveness, 0.1f, 100.0f, defaults.responsiveness);
	radius = ConfigFile::Sanitize(radius, 0.05f, 100.0f, defaults.radius);
}

void SpacePlayer::Initialize() {
	ConfigFile::Load(SpacePlayerConfig::kFileName, config_);
	position_ = { 0.0f, 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f, 0.0f };
}

void SpacePlayer::Update(float deltaTime, const Vector2& moveWorld, float vertical) {
	// ImGuiで範囲外の値が入っても破綻しないよう、毎フレーム検証する
	config_.Validate();

	const Vector3 targetVelocity = {
		moveWorld.x * config_.horizontalSpeed,
		std::clamp(vertical, -1.0f, 1.0f) * config_.verticalSpeed,
		moveWorld.y * config_.horizontalSpeed
	};

	// フレームレートに依存しない指数補間で目標速度へ近づける
	const float t = 1.0f - std::exp(-config_.responsiveness * deltaTime);
	velocity_ = velocity_ + (targetVelocity - velocity_) * t;

	position_ = position_ + velocity_ * deltaTime;
}

void SpacePlayer::DrawImGui() {
#ifdef USE_IMGUI
	config_.DrawImGui();

	ImGui::Separator();
	ImGui::Text("Position: %.2f, %.2f, %.2f", position_.x, position_.y, position_.z);
	ImGui::Text("Velocity: %.2f, %.2f, %.2f", velocity_.x, velocity_.y, velocity_.z);
	if (ImGui::Button("Reset Position")) {
		position_ = { 0.0f, 0.0f, 0.0f };
		velocity_ = { 0.0f, 0.0f, 0.0f };
	}
#endif
}

void SpacePlayer::SaveConfig() const {
	ConfigFile::Save(SpacePlayerConfig::kFileName, config_);
}
