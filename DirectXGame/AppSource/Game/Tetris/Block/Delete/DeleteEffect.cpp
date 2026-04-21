#include "DeleteEffect.h"
#include <Utility/Easing.h>

void DeleteEffect::Initialize() {
	timer_ = 0.0f;
	waitTimer_ = 0.0f;
	reqDelete_ = false;
	finishEffect_ = false;
	transform_ = {};
	transform_.rotate.z = kInitRotate_;
}

void DeleteEffect::Update(float deltaTime) {
	if (!reqDelete_) {
		//エフェクト中
		timer_ += deltaTime;
		float t = 1.0f - timer_ / kEffectingTime_;
		transform_.rotate.z = lerp(kInitRotate_, 0.0f, t, EaseType::EaseOutCubic);
		transform_.scale = { 1.0f * t, 1.0f * t, 1.0f };
		if (timer_ >= kEffectingTime_) {
			transform_.rotate.z = 0.0f;
			timer_ = kEffectingTime_;
			reqDelete_ = true;
		}
	} else if (!finishEffect_) {
		//削除待ち
		transform_.scale = { 1.0f, 1.0f, 1.0f };
		waitTimer_ += deltaTime;
		if (waitTimer_ >= kWaitTime_) {
			waitTimer_ = kWaitTime_;
			finishEffect_ = true;
		}
	}
}
