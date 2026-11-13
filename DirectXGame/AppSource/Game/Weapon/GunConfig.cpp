#include "GunConfig.h"
#include <Game/Common/ConfigFile.h>

void SimpleGunConfig::Save(BinaryManager& manager) const {
	manager.Register(fireInterval);
	manager.Register(muzzleOffset);
	manager.Register(static_cast<uint32_t>(bulletType));
	bullet.Save(manager);
}

void SimpleGunConfig::Load(BinaryManager& manager) {
	manager.Reverse(fireInterval);
	manager.Reverse(muzzleOffset);
	uint32_t type = static_cast<uint32_t>(bulletType);
	manager.Reverse(type);
	bulletType = static_cast<BulletType>(type);
	bullet.Load(manager);
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
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	simple.Load(manager);
	return true;
}

void GunConfig::Validate() {
	simple.Validate();
}
