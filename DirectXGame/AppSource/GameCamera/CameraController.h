#pragma once
#include"Camera/Camera.h"
#include"Input/Input.h"

class CameraController {
public:

	void Initialize(Input* input);

	void Update();

	void DebugDraw();

	/// <summary>
	/// マウスのスクリーン座標を3D空間の座標に変換
	/// </summary>
	/// <param name="screenPos">マウスのスクリーン座標</param>
	/// <param name="screenWidth">スクリーンの幅</param>
	/// <param name="screenHeight">スクリーンの高さ</param>
	/// <returns>3D空間上の座標</returns>
	Vector3 ScreenToWorld(const Vector2& screenPos, float screenWidth = 1280.0f, float screenHeight = 720.0f) const;

public:

	Matrix4x4 GetVpMatrix() const { return camera_->GetVPMatrix(); }
	Camera* GetCamera() const { return camera_.get(); }

	Vector3 GetWorldPos() { return worldPos_; }

private:

	Input* input_ = nullptr;

	// カメラ
	std::unique_ptr<Camera> camera_;

	// 移動速度
	float moveSpeed_ = 30.0f;

	// ワールド座標の位置
	Vector3 worldPos_ = {};
};