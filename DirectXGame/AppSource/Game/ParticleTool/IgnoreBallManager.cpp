#include "IgnoreBallManager.h"
#include <numbers>
#include <Utility/Easing.h>

void IgnoreBallManager::Initialize() {
	for (uint32_t i = 0; i < kMaxIgnoreBallNum_; ++i) {
		ignoreBalls_[i] = {};
		moveFuncs_[i] = nullptr;
	}

	static constexpr Vector3 kInitialPosition = { 0.0f, 0.0f, 20.0f };
	static constexpr float kRadius = 64.0f;

	presetFuncs_.resize(static_cast<size_t>(IgnoreBallPreset::Count));

	presetFuncs_[static_cast<size_t>(IgnoreBallPreset::None)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		PolygonEmitter::IgnoreBall ball;
		ball.position = kInitialPosition;
		ball.radius = kRadius;
		return ball;
		};

	presetFuncs_[static_cast<size_t>(IgnoreBallPreset::Round)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		PolygonEmitter::IgnoreBall ball;
		static float angle = 0.0f;
		angle = std::fmod(angle + deltaTime, 2.0f * std::numbers::pi_v<float>);
		const float roundRadius = 28.0f;
		ball.position = Vector3(roundRadius * std::cos(angle), 0.0f, roundRadius * std::sin(angle)) + kInitialPosition;
		ball.radius = kRadius;
		return ball;
		};

	presetFuncs_[static_cast<size_t>(IgnoreBallPreset::Scale)] = [](float deltaTime, bool& destroyMe) {
		destroyMe = false;
		PolygonEmitter::IgnoreBall ball;
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

	presetFuncs_[static_cast<size_t>(IgnoreBallPreset::Impact)] = [timer = 0.0f](float deltaTime, bool& destroyMe) mutable {
		destroyMe = false;

		const float kImpactTime = 0.5f;

		timer += deltaTime;
		float t = (timer / kImpactTime);

		if (timer >= kImpactTime) {
			destroyMe = true;
			t = 0.0f;
			timer = 0.0f;
		}

		PolygonEmitter::IgnoreBall ball;
		ball.position = kInitialPosition;
		ball.radius = lerp(1000.0f, 0.0f, t, EaseType::EaseInQuad);

		return ball;
		};
}

void IgnoreBallManager::Update(float deltaTime) {
	for (uint32_t i = 0; i < kMaxIgnoreBallNum_; ++i) {
		if (moveFuncs_[i]) {
			bool destroyMe = false;
			ignoreBalls_[i] = moveFuncs_[i](deltaTime, destroyMe);
			if (destroyMe) {
				moveFuncs_[i] = nullptr;
				ignoreBalls_[i] = {};
			}
		}
	}
}

uint32_t IgnoreBallManager::SetMove(std::function<PolygonEmitter::IgnoreBall(float deltaTime, bool& destroyMe)> moveFunc) {
	for (uint32_t i = 0; i < kMaxIgnoreBallNum_; ++i) {
		if (!moveFuncs_[i]) {
			moveFuncs_[i] = moveFunc;
			return i;
		}
	}
	return UINT32_MAX; // No available slot
}

void IgnoreBallManager::RemoveMove(uint32_t index) {
	if (index < kMaxIgnoreBallNum_) {
		moveFuncs_[index] = nullptr;
	}
}

void IgnoreBallManager::SetPresetFunc(IgnoreBallPreset preset) {
	SetMove(presetFuncs_[static_cast<size_t>(preset)]);
}

void IgnoreBallManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("IgnoreBallManager");
	for (int i = 0; i < kMaxIgnoreBallNum_; ++i) {
		ImGui::Text("IgnoreBall %d: Position(%.2f, %.2f, %.2f), Radius: %.2f", i, ignoreBalls_[i].position.x, ignoreBalls_[i].position.y, ignoreBalls_[i].position.z, ignoreBalls_[i].radius);
	}
	ImGui::End();
#endif
}
