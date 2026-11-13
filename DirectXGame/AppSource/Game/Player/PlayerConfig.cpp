#include "PlayerConfig.h"
#include <Game/Common/ConfigFile.h>

void PlayerConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(acceleration);
	manager.Register(damping);
	manager.Register(maxSpeed);
	manager.Register(radius);
	manager.Register(maxHp);
	manager.Register(invincibleTime);
	manager.Register(color);
}

bool PlayerConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(acceleration);
	manager.Reverse(damping);
	manager.Reverse(maxSpeed);
	manager.Reverse(radius);
	manager.Reverse(maxHp);
	manager.Reverse(invincibleTime);
	manager.Reverse(color);
	return true;
}

void PlayerConfig::Validate() {
	const PlayerConfig defaults{};
	acceleration = ConfigFile::Sanitize(acceleration, 0.0f, 1000.0f, defaults.acceleration);
	damping = ConfigFile::Sanitize(damping, 0.0001f, 1.0f, defaults.damping);
	maxSpeed = ConfigFile::Sanitize(maxSpeed, 0.01f, 1000.0f, defaults.maxSpeed);
	radius = ConfigFile::Sanitize(radius, 0.05f, 10.0f, defaults.radius);
	maxHp = ConfigFile::Sanitize(maxHp, 1, 100000);
	invincibleTime = ConfigFile::Sanitize(invincibleTime, 0.0f, 60.0f, defaults.invincibleTime);
	color = ConfigFile::SanitizeColor(color, defaults.color);
}
