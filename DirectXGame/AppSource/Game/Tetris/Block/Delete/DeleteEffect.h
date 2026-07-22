#pragma once
#include <Utility/DataStructures.h>
#include <vector>
#include <random>

class DeleteEffect {
public:

	void Initialize(int deleteLineNum);
	void Update(float deltaTime);

	const std::vector<Transform>& GetTransforms() const { return transform_; }

	//描画側に削除を要求するかどうか
	bool ReqDelete() const { return reqDelete_; }
	//演出が終了したかどうか
	bool FinishEffect() const { return finishEffect_; }

private:

	void UpdateEffect(float deltaTime);

private:

	const int kLineCount_ = 10; //1ラインのブロック数

	const float kEffectTime_ = 1.0f; //演出時間
	const float kInitSpeed_ = 20.0f; //初速
	const float kReqTime_ = 0.7f; //削除要求を出すまでの時間
	const float kMarginTime_ = 0.1f; //削除要求を出すまでの時間のマージン

private:

	std::vector<Transform> transform_{};
	std::vector<Vector3> directions_{};
	std::vector<Vector3> rotateDirections_{};
	float currentSpeed_;

	std::mt19937 randomEngine_{ std::random_device{}() };
	std::uniform_real_distribution<float> randomDist_{ -1.0f, 1.0f };

	float timer_ = 0.0f;

	bool reqDelete_ = false;
	bool finishEffect_ = false;

};

