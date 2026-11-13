#include "BulletFactory.h"
#include "SimpleBullet.h"

std::unique_ptr<Bullet> BulletFactory::Create(BulletType type, uint32_t id) const {
	switch (type) {
	case BulletType::Simple:
		return std::make_unique<SimpleBullet>(id);
	default:
		return nullptr;
	}
}
