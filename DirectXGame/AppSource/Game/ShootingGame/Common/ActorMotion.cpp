#include "ActorMotion.h"
#include "FieldConfig.h"
#include "GameMath.h"

void ActorMotion::Step(Vector2& position, Vector2& velocity, const Vector2& moveAmount, const Param& param,
	float deltaTime, const FieldConfig& field, float bodyRadius) {

	// 入力方向から加速度を作成(長さ1を超える入力は1へ制限)
	Vector2 acceleration = GameMath::ClampLength(moveAmount, 1.0f) * param.acceleration;
	acceleration = FieldRule::RemoveOutwardAcceleration(field, position, acceleration, bodyRadius);

	// 速度をdeltaTimeに応じて減衰 → 加速度を加算 → 最大速度でclamp
	velocity = GameMath::ApplyDamping(velocity, param.damping, deltaTime);
	velocity += acceleration * deltaTime;
	velocity = GameMath::ClampLength(velocity, param.maxSpeed);

	// 位置を更新し、越境していたら円周上へ戻す
	position += velocity * deltaTime;
	FieldRule::Constrain(field, position, velocity, bodyRadius);
}
