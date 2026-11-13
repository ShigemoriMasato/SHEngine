#include "BulletConfig.h"
#include <Game/Common/ConfigFile.h>

void BulletConfig::Save(BinaryManager& manager) const {
	manager.Register(speed);
	manager.Register(lifetime);
	manager.Register(radius);
	manager.Register(visualRadius);
	manager.Register(height);
	manager.Register(damage);
	manager.Register(color);
	manager.Register(destroyOnHit);
}

void BulletConfig::Load(BinaryManager& manager) {
	manager.Reverse(speed);
	manager.Reverse(lifetime);
	manager.Reverse(radius);
	manager.Reverse(visualRadius);
	manager.Reverse(height);
	manager.Reverse(damage);
	manager.Reverse(color);
	manager.Reverse(destroyOnHit);
}

void BulletConfig::Validate() {
	const BulletConfig defaults{};
	speed = ConfigFile::Sanitize(speed, 0.1f, 500.0f, defaults.speed);
	lifetime = ConfigFile::Sanitize(lifetime, 0.01f, 30.0f, defaults.lifetime);
	radius = ConfigFile::Sanitize(radius, 0.01f, 10.0f, defaults.radius);
	visualRadius = ConfigFile::Sanitize(visualRadius, 0.01f, 10.0f, defaults.visualRadius);
	height = ConfigFile::Sanitize(height, 0.0f, 100.0f, defaults.height);
	damage = ConfigFile::Sanitize(damage, 0.0f, 10000.0f, defaults.damage);
	color = ConfigFile::SanitizeColor(color, defaults.color);
}
