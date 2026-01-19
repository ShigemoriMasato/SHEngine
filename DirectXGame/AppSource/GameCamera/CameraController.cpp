#include"CameraController.h"
#include<numbers>
#include<Windows.h>
#include"DebugMousePos.h"
#include"FpsCount.h"

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

void CameraController::Initialize(Input* input) {

	// 入力処理を取得
	input_ = input;

	// カメラを初期化
	camera_ = std::make_unique<Camera>();
	camera_->SetProjectionMatrix(PerspectiveFovDesc{});
	camera_->position_ = { 0.0f,8.0f,0.0f };
	camera_->rotation_ = { -0.7f,0.0f,0.0f };
	camera_->MakeMatrix();
}

void CameraController::Update() {

	// 0:左, 1:右, 2:中
	BYTE* mouseKey = input_->GetMouseButtonState();

	// 左クリックを押している状態
	if (mouseKey && (mouseKey[0] & 0x80)) {
		// 差分を取得
		Vector2 mouseDelta = input_->GetMouseMove();
		// 移動
		camera_->position_.x += mouseDelta.x * moveSpeed_ * FpsCount::deltaTime;
		camera_->position_.z -= mouseDelta.y * moveSpeed_ * FpsCount::deltaTime;
	}

	// カメラの更新処理
	camera_->MakeMatrix();

	// ワールド座標の位置を取得する
#ifdef USE_IMGUI
	worldPos_ = ScreenToWorld(DebugMousePos::gameMousePos, 1280.0f, 720.0f);
#else
	worldPos_ = ScreenToWorld(DebugMousePos::screenMousePos, 1280.0f, 720.0f);
#endif
}

void CameraController::DebugDraw() {
	// デバック描画
	camera_->DrawImGui();

	ImGui::Begin("DebugMouse");
	ImGui::Text("GameMousePos x : %.2f, y : %.2f", DebugMousePos::gameMousePos.x, DebugMousePos::gameMousePos.y);
	ImGui::Text("WindowMousePos x : %.2f, y : %.2f", DebugMousePos::windowPos.x, DebugMousePos::windowPos.y);
	ImGui::Text("ScreenMousePos x : %.2f, y : %.2f", DebugMousePos::screenMousePos.x, DebugMousePos::screenMousePos.y);
	ImGui::Text("WorldMousePos x : %.2f, y : %.2f, z : %.2f", worldPos_.x, worldPos_.y, worldPos_.z);
	ImGui::End();
}

Vector3 CameraController::ScreenToWorld(const Vector2& screenPos, float screenWidth, float screenHeight) const {
	// 1. スクリーン座標をNDC（正規化デバイス座標）に変換
	// NDCは[-1, 1]の範囲（左下が(-1,-1)、右上が(1,1)）
	Vector2 ndc;
	ndc.x = (screenPos.x / screenWidth) * 2.0f - 1.0f;
	ndc.y = -((screenPos.y / screenHeight) * 2.0f - 1.0f); // Y軸を反転

	// 2. ビュー行列とプロジェクション行列の逆行列を取得
	Matrix4x4 viewMatrix = camera_->GetTranformMatrix();
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	
	Matrix4x4 invView = Matrix::InverseMatrix(viewMatrix);
	Matrix4x4 invProjection = Matrix::InverseMatrix(projectionMatrix);

	// 3. NDCからビュー空間への変換（Near平面とFar平面）
	Vector4 nearPoint = Vector4(ndc.x, ndc.y, 0.0f, 1.0f); // Near平面
	Vector4 farPoint = Vector4(ndc.x, ndc.y, 1.0f, 1.0f);  // Far平面

	// プロジェクション逆変換
	Vector4 nearView = {
		nearPoint.x * invProjection.m[0][0] + nearPoint.y * invProjection.m[1][0] + nearPoint.z * invProjection.m[2][0] + nearPoint.w * invProjection.m[3][0],
		nearPoint.x * invProjection.m[0][1] + nearPoint.y * invProjection.m[1][1] + nearPoint.z * invProjection.m[2][1] + nearPoint.w * invProjection.m[3][1],
		nearPoint.x * invProjection.m[0][2] + nearPoint.y * invProjection.m[1][2] + nearPoint.z * invProjection.m[2][2] + nearPoint.w * invProjection.m[3][2],
		nearPoint.x * invProjection.m[0][3] + nearPoint.y * invProjection.m[1][3] + nearPoint.z * invProjection.m[2][3] + nearPoint.w * invProjection.m[3][3]
	};
	nearView = nearView / nearView.w; // 同次座標を正規化

	Vector4 farView = {
		farPoint.x * invProjection.m[0][0] + farPoint.y * invProjection.m[1][0] + farPoint.z * invProjection.m[2][0] + farPoint.w * invProjection.m[3][0],
		farPoint.x * invProjection.m[0][1] + farPoint.y * invProjection.m[1][1] + farPoint.z * invProjection.m[2][1] + farPoint.w * invProjection.m[3][1],
		farPoint.x * invProjection.m[0][2] + farPoint.y * invProjection.m[1][2] + farPoint.z * invProjection.m[2][2] + farPoint.w * invProjection.m[3][2],
		farPoint.x * invProjection.m[0][3] + farPoint.y * invProjection.m[1][3] + farPoint.z * invProjection.m[2][3] + farPoint.w * invProjection.m[3][3]
	};
	farView = farView / farView.w; // 同次座標を正規化

	// 4. ビュー空間からワールド空間への変換
	Vector3 nearWorld = {
		nearView.x * invView.m[0][0] + nearView.y * invView.m[1][0] + nearView.z * invView.m[2][0] + invView.m[3][0],
		nearView.x * invView.m[0][1] + nearView.y * invView.m[1][1] + nearView.z * invView.m[2][1] + invView.m[3][1],
		nearView.x * invView.m[0][2] + nearView.y * invView.m[1][2] + nearView.z * invView.m[2][2] + invView.m[3][2]
	};

	Vector3 farWorld = {
		farView.x * invView.m[0][0] + farView.y * invView.m[1][0] + farView.z * invView.m[2][0] + invView.m[3][0],
		farView.x * invView.m[0][1] + farView.y * invView.m[1][1] + farView.z * invView.m[2][1] + invView.m[3][1],
		farView.x * invView.m[0][2] + farView.y * invView.m[1][2] + farView.z * invView.m[2][2] + invView.m[3][2]
	};

	// 5. レイの方向を計算
	Vector3 rayDirection = (farWorld - nearWorld).Normalize();

	// 6. レイとY=0平面の交点を求める
	// 平面の方程式: Y = 0
	// レイの方程式: P = nearWorld + t * rayDirection
	// 交点: nearWorld.y + t * rayDirection.y = 0
	// t = -nearWorld.y / rayDirection.y
	
	if (std::abs(rayDirection.y) < 0.0001f) {
		// レイが平面と平行な場合
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	float t = -nearWorld.y / rayDirection.y;
	Vector3 intersectionPoint = nearWorld + rayDirection * t;

	return intersectionPoint;
}