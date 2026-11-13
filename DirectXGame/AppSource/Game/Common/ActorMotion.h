#pragma once
#include <Utility/Vector.h>

struct FieldConfig;

/// @brief 加速度・速度・減衰・最大速度を使った移動処理。PlayerとEnemyで共通に使う。
/// 入力処理や銃の処理とは独立しており、移動方向(加速度の向きと量)だけを受け取る。
namespace ActorMotion {

	struct Param {
		float acceleration = 0.0f;	// 入力量1.0のときの加速度(ワールド単位/秒^2)
		float damping = 1.0f;		// 1秒あたりの速度残存率(0より大きく1以下)
		float maxSpeed = 0.0f;		// 最大速度(ワールド単位/秒)
	};

	/// @brief 1ステップ移動する。
	/// 処理順: 加速度作成 → 境界での外向き加速度除去 → 速度減衰(時間基準) → 加速度×dt加算 → 最大速度でclamp → 位置更新 → 越境補正
	/// @param moveAmount 加速度の向きと量(長さ0～1)。キーボードは正規化済み、スティックはアナログ量
	void Step(Vector2& position, Vector2& velocity, const Vector2& moveAmount, const Param& param,
		float deltaTime, const FieldConfig& field, float bodyRadius);
}
