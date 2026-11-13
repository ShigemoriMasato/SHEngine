#include "FieldRenderer.h"
#include <Game/Common/GameMath.h>
#include <cmath>
#include <numbers>

void FieldRenderer::Initialize(const ModelData* model) {
	renderer_.Initialize(model, kMarkerCount);
	instances_.reserve(kMarkerCount);
}

void FieldRenderer::Update(const FieldConfig& field) {
	instances_.clear();

	const float scale = kMarkerRadius / renderer_.GetModelRadius();
	for (uint32_t i = 0; i < kMarkerCount; ++i) {
		const float angle = static_cast<float>(i) / static_cast<float>(kMarkerCount) * 2.0f * std::numbers::pi_v<float>;
		const Vector2 position = field.center + Vector2(std::cos(angle), std::sin(angle)) * field.radius;

		ModelInstance instance;
		instance.world = GameMath::MakeWorldMatrix(scale, Matrix4x4::Identity(), GameMath::ToWorld(position, field.floorHeight + kMarkerRadius));
		instance.color = { 0.6f, 0.65f, 0.8f, 1.0f };
		instances_.push_back(instance);
	}

	renderer_.SetInstances(instances_);
}

void FieldRenderer::Draw(DCC* dcc, const Camera* camera) {
	renderer_.Draw(dcc, camera);
}
