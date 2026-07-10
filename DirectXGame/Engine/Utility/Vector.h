#pragma once
#include <cmath>
#include "Operator.h"

struct Vector3;
struct Vector2 final {
	float x;
	float y;

	Vector2(float x_ = 0.0f, float y_ = 0.0f) :x(x_), y(y_) {}

	float Length() const {
		return sqrtf(x * x + y * y);
	}

	Vector2 Normalize() const {
		float len = Length();
		if (len == 0.0f) {
			return { 0.0f, 0.0f };
		}
		return { x / len, y / len };
	}

	float& operator[](int i) {
		return *(&x + i);
	}
};

struct Vector3 final {
	float x;
	float y;
	float z;
	Vector3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) :x(x_), y(y_), z(z_) {}

	float Length() const {
		return sqrtf(x * x + y * y + z * z);
	}

	Vector3 Normalize() const {
		float len = Length();
		if (len == 0.0f) {
			return { 0.0f, 0.0f, 0.0f };
		}
		return { x / len, y / len, z / len };
	}

	float& operator[](int i) {
		return *(&x + i);
	}
};

struct Vector4 final {
	float x;
	float y;
	float z;
	float w;

	Vector4(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f, float w_ = 1.0f) :x(x_), y(y_), z(z_), w(w_) {}

	float Length() const {
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	Vector4 Normalize() const {
		float len = Length();
		if (len == 0.0f) {
			return { 0.0f, 0.0f, 0.0f, 0.0f };
		}
		return { x / len, y / len, z / len, w / len };
	}

	float& operator[](int i) {
		return *(&x + i);
	}
};
