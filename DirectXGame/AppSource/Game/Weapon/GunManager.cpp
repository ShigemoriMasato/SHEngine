#include "GunManager.h"
#include "SimpleGun.h"
#include <Game/Common/ConfigFile.h>

GunManager::GunManager(BulletManager* bulletManager) : bulletManager_(bulletManager) {
}

void GunManager::Initialize() {
	ConfigFile::Load(GunConfig::kFileName, config_);

	for (size_t i = 0; i < guns_.size(); ++i) {
		guns_[i] = CreateGun(static_cast<GunType>(i));
	}
}

void GunManager::Update(float deltaTime) {
	for (const auto& gun : guns_) {
		if (gun) {
			gun->Update(deltaTime);
		}
	}
}

Gun* GunManager::GetGun(GunType type) const {
	const size_t index = static_cast<size_t>(type);
	if (index >= guns_.size()) {
		return nullptr;
	}
	return guns_[index].get();
}

void GunManager::SaveConfig() const {
	ConfigFile::Save(GunConfig::kFileName, config_);
}

std::unique_ptr<Gun> GunManager::CreateGun(GunType type) const {
	switch (type) {
	case GunType::Simple:
		return std::make_unique<SimpleGun>(config_.simple, bulletManager_);
	default:
		return nullptr;
	}
}
