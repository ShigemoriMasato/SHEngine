#include "PlayerRenderer.h"
#include <Game/ShootingGame/Common/GameMath.h>
#include <cmath>

void PlayerRenderer::Initialize(const ModelData* model) {
	renderer_.Initialize(model, 1);
	instances_.resize(1);
	rollRotation_ = Matrix4x4::Identity();
	blinkTimer_ = 0.0f;
}

void PlayerRenderer::Update(const PlayerDrawState& state, float deltaTime) {
	// 球の転がり: 接地点が滑らない回転 ω = up × v / r を積算する
	const float speed = state.velocity.Length();
	if (state.isAlive && speed > GameMath::kStopSpeed && state.radius > 0.0f) {
		Vector3 axis;
		if (GameMath::TryNormalize(GameMath::Cross({ 0.0f, 1.0f, 0.0f }, state.velocity), axis)) {
			const float angle = speed * deltaTime / state.radius;
			rollRotation_ = GameMath::Orthonormalize(rollRotation_ * GameMath::MakeAxisAngleMatrix(axis, angle));
		}
	}

	// 色はモデルのマテリアル準拠。ここでは演出用の乗算色・グレースケール化だけを決める
	// 無敵中は点滅(暗くする)、ゲームオーバー時は灰色(グレースケール化して暗くする)
	Vector4 tint = { 1.0f, 1.0f, 1.0f, 1.0f };
	float grayscale = 0.0f;
	if (!state.isAlive) {
		tint = { 0.5f, 0.5f, 0.5f, 1.0f };
		grayscale = 1.0f;
		blinkTimer_ = 0.0f;
	} else if (state.isInvincible) {
		blinkTimer_ += deltaTime;
		if (std::fmod(blinkTimer_, kBlinkInterval * 2.0f) >= kBlinkInterval) {
			tint = { 0.3f, 0.3f, 0.3f, 1.0f };
		}
	} else {
		blinkTimer_ = 0.0f;
	}

	// モデルの実寸に合わせて、描画半径 = state.radius になるようスケールする
	const float scale = state.radius / renderer_.GetModelRadius();

	ModelInstance& instance = instances_.front();
	instance.world = GameMath::MakeWorldMatrix(scale, rollRotation_, state.position);
	instance.color = tint;
	instance.param = { kPatternStrength, grayscale, 0.0f, 0.0f };
	renderer_.SetInstances(instances_);
}

void PlayerRenderer::Draw(DCC* dcc, const Camera* camera) {
	renderer_.Draw(dcc, camera);
}
