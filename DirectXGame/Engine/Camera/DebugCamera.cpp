#include "DebugCamera.h"
#include "Camera.h"
#include <numbers>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

using namespace Matrix;
using namespace SHEngine;

void DebugCamera::Initialize(Input* input) {
	input_ = input;

	center_ = {0.0f, -2.0f, 0.0f};

	scale_ = Vector3(1.0f, 1.0f, 1.0f);
	position_= Vector3(0.0f, 0.0f, -20.0f);

	spherical_.x = 20.0f;
	spherical_.y = std::numbers::pi_v<float> / 2;
	spherical_.z = -std::numbers::pi_v<float> / 2;

	SetProjectionMatrix(PerspectiveFovDesc());
}

void DebugCamera::Update(bool enableInput) {

	Vector3 cameraVelocity{};
	Vector3 centerVelocity{};

	//===================
	//球面座標系
	//===================
	Vector2 mouseMove{};
	float mouseWheel = 0.0f;
	if (enableInput) {
		if (input_->GetMouseButtonState()[1] || input_->GetMouseButtonState()[2]) {
			mouseMove = input_->GetMouseMove();
			mouseMove.y *= -1.0f; // Y軸を反転
		}
		mouseWheel = -input_->GetMouseWheel();

		if (input_->GetMouseButtonState()[2] || input_->GetKeyState(DIK_LSHIFT)) {
			float speed = spherical_.x * 0.2f;
			center_ += Vector3(mouseMove.x * speed_, mouseMove.y * speed_, mouseWheel * 0.05f) * speed * MakeRotationMatrix(rotation_);
		} else {
			float moved = mouseWheel * 0.05f;
			float adjust = abs(moved) > 5.0f ? 1.0f : moved / 5.0f; // 距離が近いほど速度を遅くする
			spherical_ += Vector3(moved * adjust, mouseMove.y * speed_, -mouseMove.x * speed_);
		}
	}

	spherical_.x = std::max(0.01f, spherical_.x); // マイナスにならないようにする

	position_.x = std::sinf(spherical_.y) * std::cosf(spherical_.z);
	position_.y = std::cosf(spherical_.y);
	position_.z = std::sinf(spherical_.y) * std::sinf(spherical_.z);

	position_ *= spherical_.x; // 半径を適用

	rotation_ = { -spherical_.y + std::numbers::pi_v<float> / 2, -spherical_.z - std::numbers::pi_v<float> / 2, 0.0f };

	// Actual camera position in world space
	position_ -= center_;

	//===================
	//座標の適用
	//===================
	MakeMatrix();
}

Vector3 DebugCamera::GetCenter() const {
	return -center_;
}

void DebugCamera::MakeMatrix() {
	transformMatrix_ = MakeTranslationMatrix(-position_) * MakeRotationYMatrix(-rotation_.y) * MakeRotationXMatrix(-rotation_.x) * MakeScaleMatrix(scale_);
	vpMatrix_ = transformMatrix_ * projectionMatrix_;
}

void DebugCamera::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("DebugCamera");
	ImGui::DragFloat3("Center", &center_.x, 0.1f);
	ImGui::DragFloat3("Spherical(dist, theta, phi)", &spherical_.x, 0.1f);
	ImGui::End();
#endif
}
