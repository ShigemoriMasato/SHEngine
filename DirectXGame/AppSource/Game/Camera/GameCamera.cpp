#include "GameCamera.h"
#include <Utility/Easing.h>
#include <imgui/imgui.h>

GameCamera::~GameCamera() {
	Save();
}

void GameCamera::Initialize() {
	binaryManager_ = std::make_unique<BinaryManager>();
	camera_ = std::make_unique<Camera>();
	camera_->SetProjectionMatrix(PerspectiveFovDesc());
	Load();
}

void GameCamera::Update(float deltaTime) {

	Vector3 shakeOffset{};
	if (shakeTime_ < shakeDuration_) {
		shakeTime_ += deltaTime;
		float shakeAmount = shakeIntensity_ * (1.0f - (shakeTime_ / shakeDuration_));
		shakeOffset = {
			((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * shakeAmount,
			((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * shakeAmount,
			((static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f) * shakeAmount
		};
	}

	if (beginningT_ <= 0.0f) {
		camera_->SetTransform(Matrix::MakeAffineMatrix(Vector3(1.0f, 1.0f, 1.0f), rotation_, position_ + shakeOffset));
		camera_->MakeMatrix();
		return;
	}

	Vector3 factPos = lerp(position_ + shakeOffset, beginningPos_, beginningT_, EaseType::EaseInQuint);
	Vector3 factRot = lerp(rotation_, beginningRot_, beginningT_, EaseType::EaseInQuint);

	beginningT_ = std::max(0.0f, beginningT_ - deltaTime);

	camera_->SetTransform(Matrix::MakeAffineMatrix(Vector3(1.0f, 1.0f, 1.0f), factRot, factPos));
	camera_->MakeMatrix();
}

void GameCamera::Draw() {
}

void GameCamera::Shake(float intensity, float duration) {
	shakeTime_ = 0.0f;
	shakeDuration_ = duration;
	shakeIntensity_ = intensity;
}

void GameCamera::DrawImGui() {
#ifdef USE_IMGUI

#endif
}

void GameCamera::Save() {
	binaryManager_->Boot(saveFile_);
	binaryManager_->Register(&beginningPos_);
	binaryManager_->Register(&beginningRot_);
	binaryManager_->Write(saveFile_);
}

void GameCamera::Load() {
	if (!binaryManager_->Boot(saveFile_)) {
		return;
	}

	uint32_t index = 0;

	beginningPos_ = binaryManager_->Reverse<Vector3>();
	beginningRot_ = binaryManager_->Reverse<Vector3>();
}
