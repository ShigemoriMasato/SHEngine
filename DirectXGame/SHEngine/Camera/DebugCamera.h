#pragma once
#include "Camera.h"
#include <Input/Input.h>

/**
 * @class DebugCamera
 * @brief デバッグ用のカメラクラス
 * 
 * 球面座標系を使用して、指定した中心点の周りを回転するカメラ。
 * デバッグやシーンの確認に使用する。
 */
class DebugCamera : public Camera {
public:

	/**
	 * @brief カメラの初期化
	 */
	void Initialize(SHEngine::Input* input);

	/**
	 * @brief カメラの更新
	 * 
	 * @param enableInput 入力を受け付けるかどうか
	 * 球面座標に基づいてカメラの位置と向きを更新する。
	 */
	void Update(bool enableInput = true);

	/**
	 * @brief カメラの中心点を取得
	 * 
	 * @return 中心点の座標
	 */
	Vector3 GetCenter() const;

	/**
	 * @brief カメラの中心点を設定
	 * 
	 * @param center 設定する中心点の座標
	 */
	void SetCenter(const Vector3& center) {
		center_ = center;
	}

	/**
	 * @brief カメラの距離を設定
	 * 
	 * @param distance 中心点からの距離
	 */
	void SetDistance(float distance) {
		spherical_.x = distance;
	}

	void SetSpherical(const Vector3& spherical) {
		spherical_ = spherical;
	}

	void DrawImGui() override;

private:

	void MakeMatrix() override;

	/// @brief 入力システムへのポインタ
	SHEngine::Input* input_;

	/// @brief カメラの中心座標(カメラが注視する点)
	Vector3 center_{};

	/// @brief カメラの球面座標(x:距離, y:仰角, z:方位角)
	Vector3 spherical_{};

	/// @brief 中心からの初期距離
	float distance_ = -10.0f;

	/// @brief カメラの回転速度
	const float speed_ = 0.005f;
};
