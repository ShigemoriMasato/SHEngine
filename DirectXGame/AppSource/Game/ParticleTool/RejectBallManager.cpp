#include "RejectBallManager.h"
#include <numbers>
#include <Utility/Easing.h>

void RejectBallManager::Initialize() {
	for (uint32_t i = 0; i < kMaxRejectBallNum_; ++i) {
		rejectBalls_[i] = {};
		moveFuncs_[i] = nullptr;
	}

	static constexpr Vector3 kInitialPosition = { 0.0f, 0.0f, 20.0f };
	static constexpr float kRadius = 64.0f;

	presetFuncs_.resize(static_cast<size_t>(RejectBallPreset::Count));

	presetFuncs_[static_cast<size_t>(RejectBallPreset::None)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		RejectBallPolygonEmitter::RejectBall ball;
		ball.position = kInitialPosition;
		ball.radius = kRadius;
		return ball;
		};

	presetFuncs_[static_cast<size_t>(RejectBallPreset::Round)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		RejectBallPolygonEmitter::RejectBall ball;
		static float angle = 0.0f;
		angle = std::fmod(angle + deltaTime, 2.0f * std::numbers::pi_v<float>);
		const float roundRadius = 28.0f;
		ball.position = Vector3(roundRadius * std::cos(angle), 0.0f, roundRadius * std::sin(angle)) + kInitialPosition;
		ball.radius = kRadius;
		return ball;
		};

	presetFuncs_[static_cast<size_t>(RejectBallPreset::Scale)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		RejectBallPolygonEmitter::RejectBall ball;
		static float timer = 0.0f;
		const float cycleTime = 4.0f;
		timer = std::fmod(timer + deltaTime, cycleTime);

		const float scaleMin = 0.0f;
		const float scaleMax = 2.0f;
		float t = 1.0f - std::abs((timer / cycleTime) * 2 - 1.0f);
		float scale = lerp(scaleMin, scaleMax, t);

		ball.position = kInitialPosition;
		ball.radius = kRadius * scale;
		return ball;
		};

	presetFuncs_[static_cast<size_t>(RejectBallPreset::Impact)] = [timer = 0.0f](float deltaTime, bool& destroyMe) mutable {
		destroyMe = false;

		const float kImpactTime = 0.5f;

		timer += deltaTime;
		float t = (timer / kImpactTime);

		if (timer >= kImpactTime) {
			destroyMe = true;
			t = 0.0f;
			timer = 0.0f;
		}

		RejectBallPolygonEmitter::RejectBall ball;
		ball.position = kInitialPosition;
		ball.radius = lerp(5000.0f, 0.0f, t, EaseType::EaseInQuad);

		return ball;
		};
}

void RejectBallManager::Update(float deltaTime) {
	for (uint32_t i = 0; i < kMaxRejectBallNum_; ++i) {
		if (moveFuncs_[i]) {
			bool destroyMe = false;
			rejectBalls_[i] = moveFuncs_[i](deltaTime, destroyMe);
			if (destroyMe) {
				moveFuncs_[i] = nullptr;
				rejectBalls_[i] = {};
			}
		}
	}
}

uint32_t RejectBallManager::SetMove(std::function<RejectBallPolygonEmitter::RejectBall(float deltaTime, bool& destroyMe)> moveFunc) {
	for (uint32_t i = 0; i < kMaxRejectBallNum_; ++i) {
		if (!moveFuncs_[i]) {
			moveFuncs_[i] = moveFunc;
			return i;
		}
	}
	return UINT32_MAX; // No available slot
}

void RejectBallManager::RemoveMove(uint32_t index) {
	if (index < kMaxRejectBallNum_) {
		moveFuncs_[index] = nullptr;
	}
}

void RejectBallManager::SetPresetFunc(RejectBallPreset preset) {
	SetMove(presetFuncs_[static_cast<size_t>(preset)]);
}

void RejectBallManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("RejectBallManager");
	for (int i = 0; i < kMaxRejectBallNum_; ++i) {
		ImGui::Text("RejectBall %d: Position(%.2f, %.2f, %.2f), Radius: %.2f", i, rejectBalls_[i].position.x, rejectBalls_[i].position.y, rejectBalls_[i].position.z, rejectBalls_[i].radius);
	}
	ImGui::End();
#endif
}
