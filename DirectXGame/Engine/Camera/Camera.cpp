#include "Camera.h"
#include <Utility/MyMath.h>

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

using namespace Matrix;
using namespace MyMath;

namespace {
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
		return {
			cot(fovY / 2) / aspectRatio, 0, 0, 0,
			0, cot(fovY / 2), 0, 0,
			0, 0, farClip / (farClip - nearClip), 1,
			0, 0, (-nearClip * farClip) / (farClip - nearClip), 0
		};
	}

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
		return {
			2 / (right - left), 0, 0, 0,
			0, 2 / (top - bottom), 0, 0,
			0, 0, 1 / (farClip - nearClip), 0,
			(left + right) / (left - right), (top + bottom) / (bottom - top), nearClip / (nearClip - farClip), 1
		};
	}

	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
		return {
			width / 2, 0, 0, 0,
			0, -height / 2, 0, 0,
			0, 0, maxDepth - minDepth, 0,
			left + width / 2, top + height / 2, minDepth, 1
		};
	}
}

Camera::Camera() {
	vpBuffer_ = std::make_unique<SHEngine::GPUBuffer>(BufferType::CBV, sizeof(Matrix4x4));
}

void Camera::SetProjectionMatrix(PerspectiveFovDesc desc) {
	projectionMatrix_ = MakePerspectiveFovMatrix(desc.fovY, desc.aspectRatio, desc.nearClip, desc.farClip);
}

void Camera::SetProjectionMatrix(OrthographicDesc desc) {
	projectionMatrix_ = MakeOrthographicMatrix(desc.left, desc.top, desc.right, desc.bottom, desc.nearClip, desc.farClip);
}

bool Camera::UpdateCurve(float deltaTime, bool repeat) {
	if (curveData_.totalTime <= 0.0f) {
		return false;
	}

	bool ans = false;
	
	timer_ += deltaTime;
	if (timer_ > curveData_.totalTime) {
		if (repeat) {
			timer_ -= curveData_.totalTime;
		} else {
			timer_ = curveData_.totalTime;
		}

		ans = true;
	}
	
	position_.x = curveData_.posXCurve.Evaluate(timer_);
	position_.y = curveData_.posYCurve.Evaluate(timer_);
	position_.z = curveData_.posZCurve.Evaluate(timer_);

	rotation_.x = curveData_.rotXCurve.Evaluate(timer_);
	rotation_.y = curveData_.rotYCurve.Evaluate(timer_);
	rotation_.z = curveData_.rotZCurve.Evaluate(timer_);

	MakeMatrix();

	return ans;
}

void Camera::MakeMatrix() {
	if (!isSetMatrix) {
		transformMatrix_ = MakeTranslationMatrix(-position_) * MakeRotationMatrix(rotation_) * MakeScaleMatrix(scale_);
	}
	vpMatrix_ = transformMatrix_ * projectionMatrix_;
	vpBuffer_->CopyBuffer(&vpMatrix_, sizeof(Matrix4x4));
}

void Camera::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Position", &position_.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotation_.x, 0.01f);
	ImGui::DragFloat3("Scale", &scale_.x, 0.1f);
	ImGui::End();
#endif
}

void Camera::SetTransform(Matrix4x4 mat) {
	transformMatrix_ = mat;
	isSetMatrix = true;
}

Matrix4x4 Camera::GetVPMatrix() const {
	return vpMatrix_;
}

Matrix4x4 Camera::GetBillboardMatrix() const {
	Matrix4x4 billboardMatrix = transformMatrix_;
	//平行移動成分を削除
	for (int i = 0; i < 3; ++i) {
		billboardMatrix.m[3][i] = 0.0f;
	}
	return billboardMatrix.Inverse();
}

Vector3 Camera::GetDirection() const {
	//角度ゼロの時を基準にして、カメラの向きを計算する
	Vector3 direction = { 0.0f, 0.0f, 1.0f };
	direction = MakeRotationMatrix(rotation_) * direction;
	return direction.Normalize();
}

void Camera::Inport(const CameraCurveData& data) {
	curveData_ = data;
}
