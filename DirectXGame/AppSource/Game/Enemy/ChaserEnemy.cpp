#include "ChaserEnemy.h"
#include <Game/Common/GameMath.h>

ChaserEnemy::ChaserEnemy(uint32_t id, const FieldConfig* field, const ChaserEnemyConfig& config)
	: Enemy(EnemyType::Chaser, id, field), config_(config) {
	hp_ = config_.maxHp;
	radius_ = config_.radius;
	color_ = config_.color;
}

void ChaserEnemy::Update(float deltaTime, const EnemyTarget& target) {
	if (isDead_) {
		return;
	}

	// 追跡対象への方向を毎フレーム求める。同じ位置にいる場合は加速しない(ゼロベクトルを正規化しない)
	Vector2 direction = { 0.0f, 0.0f };
	if (!GameMath::TryNormalize(target.position - position_, direction)) {
		direction = { 0.0f, 0.0f };
	}

	ActorMotion::Param param;
	param.acceleration = config_.acceleration;
	param.damping = config_.damping;
	param.maxSpeed = config_.maxSpeed;
	Move(deltaTime, direction, param);
}
