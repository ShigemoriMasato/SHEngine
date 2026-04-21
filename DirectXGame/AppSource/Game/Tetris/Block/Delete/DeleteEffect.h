#pragma once
#include <Utility/DataStructures.h>

class DeleteEffect {
public:

	void Initialize();
	void Update(float deltaTime);

	Transform GetTransform() const { return transform_; }

	bool ReqDelete() const { return reqDelete_; }
	bool FinishEffect() const { return finishEffect_; }

private:

	const float kEffectingTime_ = 0.5f;
	const float kWaitTime_ = 0.2f;

	const float kInitRotate_ = 30.0f;

private:

	Transform transform_{};
	float timer_ = 0.0f;
	float waitTimer_ = 0.0f;

	bool reqDelete_ = false;
	bool finishEffect_ = false;

};

