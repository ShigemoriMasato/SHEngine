#pragma once
#include <Utility/Vector.h>
#include <Utility/Quaternion.h>
#include <Utility/MatrixFactory.h>

struct Transform {
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Vector3 rotate{};
	Vector3 position{};

	Matrix4x4 Matrix() const {
		return Matrix::MakeScaleMatrix(scale) * Matrix::MakeRotationMatrix(rotate) * Matrix::MakeTranslationMatrix(position);
	}
};

struct QuaternionTransform {
	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Quaternion rotate{};
	Vector3 position{};

	Matrix4x4 Matrix() const {
		return Matrix::MakeScaleMatrix(scale) * rotate.ToMatrix() * Matrix::MakeTranslationMatrix(position);
	}
};

struct DirectionalLight {
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector3 direction = { 0.0f, -1.0f, 0.0f };
	float intensity = 1.0f;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	Vector2 padding;
};
