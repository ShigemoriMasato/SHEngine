#include "PlayerConfig.h"
#include <Game/ShootingGame/Common/ConfigFile.h>
#include <imgui/imgui.h>

void PlayerConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(acceleration);
	manager.Register(damping);
	manager.Register(maxSpeed);
	manager.Register(maxHp);
	manager.Register(invincibleTime);
	manager.Register(specialGaugeMax);
}

bool PlayerConfig::Load(BinaryManager& manager) {
	uint32_t version = 0;
	if (!ConfigFile::ReadVersion(manager, kLegacyColorVersion, kVersion, version)) {
		return false;
	}
	manager.Reverse(acceleration);
	manager.Reverse(damping);
	manager.Reverse(maxSpeed);
	manager.Reverse(maxHp);
	manager.Reverse(invincibleTime);
	manager.Reverse(specialGaugeMax);
	if (version == kLegacyColorVersion) {
		Vector4 legacyColor{};
		manager.Reverse(legacyColor);
	}
	return true;
}

void PlayerConfig::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::DragFloat("Acceleration", &acceleration, 0.1f);
	ImGui::DragFloat("Damping", &damping, 0.001f);
	ImGui::DragFloat("Max Speed", &maxSpeed, 0.1f);
	ImGui::DragInt("Max HP", &maxHp, 1);
	ImGui::DragFloat("Invincible Time", &invincibleTime, 0.1f);
	ImGui::DragInt("Special Gauge Max", &specialGaugeMax, 1);
	ImGui::Checkbox("Is Invincible", &isInvincible);

#endif
}

void PlayerConfig::Validate() {
	const PlayerConfig defaults{};
	acceleration = ConfigFile::Sanitize(acceleration, 0.0f, 1000.0f, defaults.acceleration);
	damping = ConfigFile::Sanitize(damping, 0.0001f, 1.0f, defaults.damping);
	maxSpeed = ConfigFile::Sanitize(maxSpeed, 0.01f, 1000.0f, defaults.maxSpeed);
	maxHp = ConfigFile::Sanitize(maxHp, 1, 100000);
	invincibleTime = ConfigFile::Sanitize(invincibleTime, 0.0f, 60.0f, defaults.invincibleTime);
	specialGaugeMax = ConfigFile::Sanitize(specialGaugeMax, 1, 10000);
}
