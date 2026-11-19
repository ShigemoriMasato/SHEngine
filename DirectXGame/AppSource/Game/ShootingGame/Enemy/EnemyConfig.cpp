#include "EnemyConfig.h"
#include <Game/ShootingGame/Common/ConfigFile.h>

void ChaserEnemyConfig::Save(BinaryManager& manager) const {
	manager.Register(acceleration);
	manager.Register(damping);
	manager.Register(maxSpeed);
	manager.Register(radius);
	manager.Register(maxHp);
	manager.Register(contactDamage);
}

void ChaserEnemyConfig::Load(BinaryManager& manager, bool hasLegacyColor) {
	manager.Reverse(acceleration);
	manager.Reverse(damping);
	manager.Reverse(maxSpeed);
	manager.Reverse(radius);
	manager.Reverse(maxHp);
	manager.Reverse(contactDamage);
	if (hasLegacyColor) {
		Vector4 legacyColor{};
		manager.Reverse(legacyColor);
	}
}

void ChaserEnemyConfig::Validate() {
	const ChaserEnemyConfig defaults{};
	acceleration = ConfigFile::Sanitize(acceleration, 0.0f, 1000.0f, defaults.acceleration);
	damping = ConfigFile::Sanitize(damping, 0.0001f, 1.0f, defaults.damping);
	maxSpeed = ConfigFile::Sanitize(maxSpeed, 0.01f, 1000.0f, defaults.maxSpeed);
	radius = ConfigFile::Sanitize(radius, 0.05f, 10.0f, defaults.radius);
	maxHp = ConfigFile::Sanitize(maxHp, 0.01f, 100000.0f, defaults.maxHp);
	contactDamage = ConfigFile::Sanitize(contactDamage, 0, 100000);
}

void EnemySpawnConfig::Save(BinaryManager& manager) const {
	manager.Register(initialDelay);
	manager.Register(interval);
	manager.Register(maxCount);
	manager.Register(minDistanceFromPlayer);
	manager.Register(maxSpawnAttempts);
}

void EnemySpawnConfig::Load(BinaryManager& manager) {
	manager.Reverse(initialDelay);
	manager.Reverse(interval);
	manager.Reverse(maxCount);
	manager.Reverse(minDistanceFromPlayer);
	manager.Reverse(maxSpawnAttempts);
}

void EnemySpawnConfig::Validate() {
	const EnemySpawnConfig defaults{};
	initialDelay = ConfigFile::Sanitize(initialDelay, 0.0f, 600.0f, defaults.initialDelay);
	interval = ConfigFile::Sanitize(interval, 0.05f, 600.0f, defaults.interval);
	maxCount = ConfigFile::Sanitize(maxCount, 1u, 256u);
	minDistanceFromPlayer = ConfigFile::Sanitize(minDistanceFromPlayer, 0.0f, 1000.0f, defaults.minDistanceFromPlayer);
	maxSpawnAttempts = ConfigFile::Sanitize(maxSpawnAttempts, 1u, 64u);
}

void EnemyConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	chaser.Save(manager);
	spawn.Save(manager);
}

bool EnemyConfig::Load(BinaryManager& manager) {
	uint32_t version = 0;
	if (!ConfigFile::ReadVersion(manager, kLegacyColorVersion, kVersion, version)) {
		return false;
	}
	chaser.Load(manager, version == kLegacyColorVersion);
	spawn.Load(manager);
	return true;
}

void EnemyConfig::Validate() {
	chaser.Validate();
	spawn.Validate();
}
