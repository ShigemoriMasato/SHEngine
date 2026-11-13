#include "FieldConfig.h"
#include "ConfigFile.h"
#include "GameMath.h"
#include <algorithm>

namespace {
	// 境界上とみなす距離の許容誤差(ワールド単位)
	constexpr float kBoundaryEpsilon = 1.0e-4f;
}

void FieldConfig::Save(BinaryManager& manager) const {
	manager.Register(kVersion);
	manager.Register(center);
	manager.Register(radius);
	manager.Register(floorHeight);
}

bool FieldConfig::Load(BinaryManager& manager) {
	if (!ConfigFile::ReadVersion(manager, kVersion)) {
		return false;
	}
	manager.Reverse(center);
	manager.Reverse(radius);
	manager.Reverse(floorHeight);
	return true;
}

void FieldConfig::Validate() {
	const FieldConfig defaults{};
	center = ConfigFile::Sanitize(center, defaults.center);
	radius = ConfigFile::Sanitize(radius, 1.0f, 1000.0f, defaults.radius);
	floorHeight = ConfigFile::Sanitize(floorHeight, -1000.0f, 1000.0f, defaults.floorHeight);
}

float FieldRule::GetMovableRadius(const FieldConfig& field, float bodyRadius) {
	return std::max(0.0f, field.radius - bodyRadius);
}

Vector2 FieldRule::RemoveOutwardAcceleration(const FieldConfig& field, const Vector2& position, const Vector2& acceleration, float bodyRadius) {
	const Vector2 offset = position - field.center;
	const float limit = GetMovableRadius(field, bodyRadius);

	if (offset.Length() < limit - kBoundaryEpsilon) {
		return acceleration;
	}

	Vector2 normal;
	if (!GameMath::TryNormalize(offset, normal)) {
		return acceleration;
	}

	const float outward = GameMath::Dot(acceleration, normal);
	if (outward <= 0.0f) {
		return acceleration;
	}
	return acceleration - normal * outward;
}

bool FieldRule::Constrain(const FieldConfig& field, Vector2& position, Vector2& velocity, float bodyRadius) {
	const Vector2 offset = position - field.center;
	const float limit = GetMovableRadius(field, bodyRadius);

	if (offset.Length() <= limit) {
		return false;
	}

	Vector2 normal;
	if (!GameMath::TryNormalize(offset, normal)) {
		return false;
	}

	position = field.center + normal * limit;

	const float outward = GameMath::Dot(velocity, normal);
	if (outward > 0.0f) {
		velocity = velocity - normal * outward;
	}
	return true;
}

bool FieldRule::IsOutside(const FieldConfig& field, const Vector2& position, float margin) {
	return (position - field.center).Length() > field.radius + margin;
}
