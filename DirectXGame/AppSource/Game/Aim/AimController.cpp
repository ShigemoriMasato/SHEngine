#include "AimController.h"
#include <Game/Common/ConfigFile.h>
#include <Game/Common/GameMath.h>
#include <cmath>

void AimConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(mouseDeadRadius);
	manager.Register(initialDirection);
}

bool AimConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(mouseDeadRadius);
	manager.Reverse(initialDirection);
	return true;
}

void AimConfig::Validate() {
	const AimConfig defaults{};
	mouseDeadRadius = ConfigFile::Sanitize(mouseDeadRadius, 0.0f, 200.0f, defaults.mouseDeadRadius);
	Vector2 direction;
	if (!GameMath::TryNormalize(ConfigFile::Sanitize(initialDirection, defaults.initialDirection), direction)) {
		direction = defaults.initialDirection;
	}
	initialDirection = direction;
}

void AimController::Initialize() {
	ConfigFile::Load(AimConfig::kFileName, config_);
	direction_ = config_.initialDirection;
}

void AimController::Update(const Vector3& playerWorldPosition, const Matrix4x4& viewMatrix, const Matrix4x4& viewProjectionMatrix, const AimInput& input) {
	Vector2 direction;
	bool updated = false;

	if (input.device == InputDevice::Controller) {
		// デッドゾーン内(ゼロ)なら直前の照準を維持する
		if (input.rightStick.Length() > 0.0f) {
			updated = ScreenVectorToWorld(input.rightStick, viewMatrix, direction);
		}
	} else {
		updated = TryMouseDirection(playerWorldPosition, viewMatrix, viewProjectionMatrix, input, direction);
	}

	if (updated) {
		direction_ = direction;
	}
}

void AimController::SaveConfig() const {
	ConfigFile::Save(AimConfig::kFileName, config_);
}

bool AimController::TryMouseDirection(const Vector3& playerWorldPosition, const Matrix4x4& viewMatrix, const Matrix4x4& viewProjectionMatrix,
	const AimInput& input, Vector2& out) const {

	if (input.screenSize.x <= 0.0f || input.screenSize.y <= 0.0f) {
		return false;
	}

	// プレイヤーの画面位置(ピクセル)
	Vector2 playerScreen;
	bool hasPlayerScreen = false;
	{
		Vector3 ndc;
		if (GameMath::TryTransformPoint(playerWorldPosition, viewProjectionMatrix, ndc)) {
			playerScreen = {
				(ndc.x * 0.5f + 0.5f) * input.screenSize.x,
				(0.5f - ndc.y * 0.5f) * input.screenSize.y
			};
			hasPlayerScreen = true;

			// カーソルがプレイヤーに重なっている間は方向が定まらないため維持する
			if ((input.cursorPosition - playerScreen).Length() <= config_.mouseDeadRadius) {
				return false;
			}
		}
	}

	// カーソル位置のレイとプレイヤー高さの水平面の交点を求める
	const Vector2 cursorNdc = {
		input.cursorPosition.x / input.screenSize.x * 2.0f - 1.0f,
		1.0f - input.cursorPosition.y / input.screenSize.y * 2.0f
	};
	const Matrix4x4 inverseViewProjection = viewProjectionMatrix.Inverse();

	Vector3 nearPoint;
	Vector3 farPoint;
	if (GameMath::TryTransformPoint({ cursorNdc.x, cursorNdc.y, 0.0f }, inverseViewProjection, nearPoint) &&
		GameMath::TryTransformPoint({ cursorNdc.x, cursorNdc.y, 1.0f }, inverseViewProjection, farPoint)) {

		const float denominator = farPoint.y - nearPoint.y;
		if (std::fabs(denominator) > 1.0e-6f) {
			const float t = (playerWorldPosition.y - nearPoint.y) / denominator;
			if (t >= 0.0f && std::isfinite(t)) {
				const Vector3 hit = nearPoint + (farPoint - nearPoint) * t;
				const Vector2 toHit = { hit.x - playerWorldPosition.x, hit.z - playerWorldPosition.z };
				if (GameMath::TryNormalize(toHit, out)) {
					return true;
				}
			}
		}
	}

	// 平面と交差できない場合は、画面上の差をカメラ基準でXZへ変換する
	if (!hasPlayerScreen) {
		return false;
	}
	const Vector2 screenDelta = {
		input.cursorPosition.x - playerScreen.x,
		playerScreen.y - input.cursorPosition.y	// 画面上方向を正にする
	};
	return ScreenVectorToWorld(screenDelta, viewMatrix, out);
}

bool AimController::ScreenVectorToWorld(const Vector2& screenVector, const Matrix4x4& viewMatrix, Vector2& out) {
	// ビュー行列(行ベクトル規約)の列0がカメラ右、列1がカメラ上、列2がカメラ前方向
	Vector2 rightXZ;
	if (!GameMath::TryNormalize(Vector2(viewMatrix.m[0][0], viewMatrix.m[2][0]), rightXZ)) {
		return false;
	}

	// 画面上方向はカメラの上方向をXZへ投影したもの。真横から見ている場合は前方向で代用する
	Vector2 upXZ;
	if (!GameMath::TryNormalize(Vector2(viewMatrix.m[0][1], viewMatrix.m[2][1]), upXZ) &&
		!GameMath::TryNormalize(Vector2(viewMatrix.m[0][2], viewMatrix.m[2][2]), upXZ)) {
		return false;
	}

	return GameMath::TryNormalize(rightXZ * screenVector.x + upXZ * screenVector.y, out);
}
