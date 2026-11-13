#include "ConfigFile.h"
#include <algorithm>
#include <cmath>

bool ConfigFile::ReadVersion(BinaryManager& manager, uint32_t expected) {
	uint32_t version = 0;
	manager.Reverse(version);
	return version == expected;
}

float ConfigFile::Sanitize(float value, float minValue, float maxValue, float fallback) {
	if (!std::isfinite(value)) {
		return fallback;
	}
	return std::clamp(value, minValue, maxValue);
}

Vector2 ConfigFile::Sanitize(const Vector2& value, const Vector2& fallback) {
	if (!std::isfinite(value.x) || !std::isfinite(value.y)) {
		return fallback;
	}
	return value;
}

Vector4 ConfigFile::SanitizeColor(const Vector4& value, const Vector4& fallback) {
	if (!std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z) || !std::isfinite(value.w)) {
		return fallback;
	}
	return {
		std::clamp(value.x, 0.0f, 1.0f),
		std::clamp(value.y, 0.0f, 1.0f),
		std::clamp(value.z, 0.0f, 1.0f),
		std::clamp(value.w, 0.0f, 1.0f)
	};
}

int32_t ConfigFile::Sanitize(int32_t value, int32_t minValue, int32_t maxValue) {
	return std::clamp(value, minValue, maxValue);
}

uint32_t ConfigFile::Sanitize(uint32_t value, uint32_t minValue, uint32_t maxValue) {
	return std::clamp(value, minValue, maxValue);
}
