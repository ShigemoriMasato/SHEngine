#pragma once
#include"Camera/Camera.h"
#include"Input/Input.h"

class CameraController : public Camera {
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

	Matrix4x4 GetVpMatrix() const { return vpMatrix_; }

	Vector3 GetWorldPos() { return worldPos_; }

private:

	void MakeMatrix() override;

	Vector3 backDir_ = { 0.0f,-0.84f,0.55f };
	float backDist_ = 10.0f;

	Input* input_ = nullptr;
	//クリックした瞬間のマウス座標
	Vector3 clickedCameraPos_ = {};
	Vector2 mousePos_ = {};
	bool preClicked_ = false;
	float deadZone_ = 10.0f;

	// 移動速度
	float moveSpeed_ = 30.0f;

	// ワールド座標の位置
	Vector3 worldPos_ = {};
};