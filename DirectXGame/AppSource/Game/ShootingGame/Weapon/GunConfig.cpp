#include "GunConfig.h"
#include <Game/ShootingGame/Common/ConfigFile.h>

void SimpleGunConfig::Save(BinaryManager& manager) const {
	manager.Register(fireInterval);
	manager.Register(muzzleOffset);
	manager.Register(static_cast<uint32_t>(bulletType));
	bullet.Save(manager);
}

void SimpleGunConfig::Load(BinaryManager& manager, bool hasLegacyColor) {
	manager.Reverse(fireInterval);
	manager.Reverse(muzzleOffset);
	uint32_t type = static_cast<uint32_t>(bulletType);
	manager.Reverse(type);
	bulletType = static_cast<BulletType>(type);
	bullet.Load(manager, hasLegacyColor);
}

void SimpleGunConfig::Validate() {
	const SimpleGunConfig defaults{};
	fireInterval = ConfigFile::Sanitize(fireInterval, 0.01f, 10.0f, defaults.fireInterval);
	muzzleOffset = ConfigFile::Sanitize(muzzleOffset, defaults.muzzleOffset);
	if (static_cast<uint32_t>(bulletType) >= static_cast<uint32_t>(BulletType::Count)) {
		bulletType = defaults.bulletType;
	}
	bullet.Validate();
}

void GunConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	simple.Save(manager);
}

bool GunConfig::Load(BinaryManager& manager) {
	uint32_t version = 0;
	if (!ConfigFile::ReadVersion(manager, kLegacyColorVersion, kVersion, version)) {
		return false;
	}
	simple.Load(manager, version == kLegacyColorVersion);
	return true;
}

void GunConfig::Validate() {
	simple.Validate();
}
