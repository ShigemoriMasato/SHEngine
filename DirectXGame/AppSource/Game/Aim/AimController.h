#pragma once
#include <Common/KeyConfig/InputDevice.h>
#include <Utility/Matrix.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief 照準の調整値。反映タイミング: 即時(次のUpdateから)
struct AimConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "AimConfig.bin";

	// カーソルがプレイヤーの画面位置からこの距離(ピクセル)以内の場合、方向が不安定になるため直前の照準を維持する(0～200)
	float mouseDeadRadius = 8.0f;
	// 照準入力がまだ無いときの照準方向(XZ)
	Vector2 initialDirection = { 0.0f, 1.0f };

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void Validate();
};

/// @brief 照準計算の入力。GameSceneがKeyManagerから作る
struct AimInput {
	InputDevice device = InputDevice::KeyboardMouse;
	Vector2 cursorPosition = {};				// 描画先基準のピクセル座標(左上原点, y下向き)
	Vector2 rightStick = {};					// デッドゾーン適用後の右スティック(x右, y上, 長さ0～1)
	Vector2 screenSize = { 1280.0f, 720.0f };	// 描画先の解像度
};

/// @brief 照準専用クラス。カメラ行列はこのクラスにだけ渡し、XZ平面上のワールド照準方向をFireContext用に提供する。
/// マウス: カーソル位置からのレイと、プレイヤー高さの水平面との交点へ向かう方向。交差できない場合は
///         プレイヤーの画面位置とカーソルの差をカメラ基準でXZへ変換する。
/// コントローラー: 右スティックを画面上のベクトルとして扱い、カメラ基準でXZへ変換する。
/// どちらもゼロ方向・交差失敗時は直前の照準方向を維持する。
class AimController {
public:

	void Initialize();

	/// @brief (4) 更新後のカメラ行列から照準方向を求める
	void Update(const Vector3& playerWorldPosition, const Matrix4x4& viewMatrix, const Matrix4x4& viewProjectionMatrix, const AimInput& input);

	/// @brief 照準方向(XZ, 単位ベクトル)
	const Vector2& GetDirection() const { return direction_; }

	void SaveConfig() const;

private:

	bool TryMouseDirection(const Vector3& playerWorldPosition, const Matrix4x4& viewMatrix, const Matrix4x4& viewProjectionMatrix,
		const AimInput& input, Vector2& out) const;

	/// @brief 画面上のベクトル(x右, y上)をカメラ基準でXZ平面の方向へ変換する
	static bool ScreenVectorToWorld(const Vector2& screenVector, const Matrix4x4& viewMatrix, Vector2& out);

	AimConfig config_{};
	Vector2 direction_ = { 0.0f, 1.0f };
};
