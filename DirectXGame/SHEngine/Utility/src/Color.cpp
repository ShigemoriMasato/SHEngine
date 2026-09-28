#include "../Color.h"

Vector4 ConvertColor(uint32_t color) {
	float r = ((color >> 24) & 0xFF) / 255.0f;
	float g = ((color >> 16) & 0xFF) / 255.0f;
	float b = ((color >> 8 ) & 0xFF) / 255.0f;
	float a = ((color >> 0 ) & 0xFF) / 255.0f;
	return { r, g, b, a };
}

uint32_t ConvertColor(const Vector4& color) {
	uint32_t r = static_cast<uint32_t>(color.x * 255.0f) & 0xFF;
	uint32_t g = static_cast<uint32_t>(color.y * 255.0f) & 0xFF;
	uint32_t b = static_cast<uint32_t>(color.z * 255.0f) & 0xFF;
	uint32_t a = static_cast<uint32_t>(color.w * 255.0f) & 0xFF;
	return (r << 24) | (g << 16) | (b << 8) | a;
}

Vector4 ConvertColorForImGui(uint32_t color) {
	float a = ((color >> 24) & 0xFF) / 255.0f;
	float b = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >> 8) & 0xFF) / 255.0f;
	float r = ((color >> 0) & 0xFF) / 255.0f;
	return { r, g, b, a };
}

uint32_t ConvertColorForImGui(const Vector4& color) {
	uint32_t r = static_cast<uint32_t>(color.w * 255.0f) & 0xFF;
	uint32_t g = static_cast<uint32_t>(color.z * 255.0f) & 0xFF;
	uint32_t b = static_cast<uint32_t>(color.y * 255.0f) & 0xFF;
	uint32_t a = static_cast<uint32_t>(color.x * 255.0f) & 0xFF;
	return (r << 24) | (g << 16) | (b << 8) | a;
}
