#pragma once
#include "Enemy.h"
#include "EnemyConfig.h"

/// @brief 追跡対象へ向かって加速し、突撃する敵。接触時に contactDamage を要求する
class ChaserEnemy : public Enemy {
public:

	ChaserEnemy(uint32_t id, const FieldConfig* field, const ChaserEnemyConfig& config);

	void Update(float deltaTime, const EnemyTarget& target) override;
	int32_t GetContactDamage() const override { return config_.contactDamage; }

private:

	ChaserEnemyConfig config_;
};
