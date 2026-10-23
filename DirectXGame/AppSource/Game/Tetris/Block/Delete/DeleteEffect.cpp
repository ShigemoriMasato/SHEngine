#include "DeleteEffect.h"
#include <Utility/Easing.h>
#include <imgui/imgui.h>

void DeleteEffect::Initialize(int deleteLineNum) {
	timer_ = 0.0f;
	reqDelete_ = false;
	finishEffect_ = false;
	currentSpeed_ = kInitSpeed_;

	uint32_t blockNum = deleteLineNum * kLineCount_;
	transform_.resize(blockNum);
	directions_.resize(blockNum);
	rotateDirections_.resize(blockNum);
	for (uint32_t i = 0; i < blockNum; ++i) {
		directions_[i] = Vector3(randomDist_(randomEngine_), randomDist_(randomEngine_), randomDist_(randomEngine_)).Normalize();
		rotateDirections_[i] = Vector3(randomDist_(randomEngine_), randomDist_(randomEngine_), randomDist_(randomEngine_)).Normalize();
	}
}

void DeleteEffect::Update(float deltaTime) {
	if (!reqDelete_) {
		//エフェクト中
		timer_ += deltaTime;
		UpdateEffect(deltaTime);
		if (timer_ >= kReqTime_) {
			reqDelete_ = true;
		}
	} else if (!finishEffect_) {
		//削除待ち
		timer_ += deltaTime;
		if (timer_ >= kEffectTime_) {
			finishEffect_ = true;
		}
	}
}

void DeleteEffect::UpdateEffect(float deltaTime) {
	float scale = timer_ / (kEffectTime_ / 1.f) + 1.0f;

	for (int i = 0; i < int(transform_.size()); ++i) {
		transform_[i].scale = Vector3(scale, scale, scale);
		//transform_[i].rotate += rotateDirections_[i] * currentSpeed_ * deltaTime;
	}
}
