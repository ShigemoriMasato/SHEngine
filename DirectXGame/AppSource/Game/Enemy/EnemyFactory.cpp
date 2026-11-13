#include "EnemyFactory.h"
#include "ChaserEnemy.h"

std::unique_ptr<Enemy> EnemyFactory::Create(EnemyType type, uint32_t id, const EnemyConfig& config, const FieldConfig* field) const {
	switch (type) {
	case EnemyType::Chaser:
		return std::make_unique<ChaserEnemy>(id, field, config.chaser);
	default:
		return nullptr;
	}
}
