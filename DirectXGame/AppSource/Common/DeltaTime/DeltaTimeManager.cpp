#include "DeltaTimeManager.h"
#include <algorithm>
#include <imgui/imgui.h>

DeltaTimeManager::DeltaTimeManager() {
	binder_.BindFloat("Ratio", &ratio_);

	editor_.SetBinder(&binder_);
	player_.SetBinder(&binder_);
}

void DeltaTimeManager::Boot(std::string itemName) {
	if (player_.Load(kCategory_, itemName)) {
		player_.Play();
	}
}

void DeltaTimeManager::Update(float deltaTime) {
	float clampedDeltaTime = std::clamp(deltaTime, 0.0f, 1.0f / 20.0f);
	player_.Update(clampedDeltaTime);
	editor_.Update(clampedDeltaTime);

	if (player_.IsFinished()) {
		ratio_ = 1.0f;
	}

	deltaTime_ = clampedDeltaTime * ratio_;
}

void DeltaTimeManager::DrawImGui() {
#ifdef USE_IMGUI
	editor_.DrawImGui();
#endif
}
