#include "GameMath.h"
#include <cmath>

float GameMath::Dot(const Vector2& a, const Vector2& b) {
	return a.x * b.x + a.y * b.y;
}

float GameMath::Dot(const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 GameMath::Cross(const Vector3& a, const Vector3& b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

bool GameMath::TryNormalize(const Vector2& v, Vector2& out) {
	float length = v.Length();
	if (!std::isfinite(length) || length <= kDirectionEpsilon) {
		return false;
	}
	out = { v.x / length, v.y / length };
	return true;
}

bool GameMath::TryNormalize(const Vector3& v, Vector3& out) {
	float length = v.Length();
	if (!std::isfinite(length) || length <= kDirectionEpsilon) {
		return false;
	}
	out = { v.x / length, v.y / length, v.z / length };
	return true;
}

Vector2 GameMath::ClampLength(const Vector2& v, float maxLength) {
	float length = v.Length();
	if (length <= maxLength || length <= 0.0f) {
		return v;
	}
	if (maxLength <= 0.0f) {
		return { 0.0f, 0.0f };
	}
	return v * (maxLength / length);
}

Vector2 GameMath::ApplyDamping(const Vector2& velocity, float damping, float deltaTime) {
	Vector2 result = velocity * std::pow(damping, deltaTime);
	if (result.Length() < kStopSpeed) {
		return { 0.0f, 0.0f };
	}
	return result;
}

Vector3 GameMath::ToWorld(const Vector2& xz, float height) {
	return { xz.x, height, xz.y };
}

bool GameMath::IsFinite(float value) {
	return std::isfinite(value);
}

bool GameMath::IsFinite(const Vector2& v) {
	return std::isfinite(v.x) && std::isfinite(v.y);
}

bool GameMath::IsFinite(const Vector4& v) {
	return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && std::isfinite(v.w);
}

Matrix4x4 GameMath::MakeAxisAngleMatrix(const Vector3& k, float angle) {
	// ロドリゲスの回転公式を行ベクトル規約で展開したもの。
	// 行iは基底e_iの回転後の像 = e_i cos + (k × e_i) sin + k k_i (1 - cos)
	const float c = std::cos(angle);
	const float s = std::sin(angle);
	const float t = 1.0f - c;

	return {
		c + t * k.x * k.x,			s * k.z + t * k.x * k.y,	-s * k.y + t * k.x * k.z,	0.0f,
		-s * k.z + t * k.x * k.y,	c + t * k.y * k.y,			s * k.x + t * k.y * k.z,	0.0f,
		s * k.y + t * k.x * k.z,	-s * k.x + t * k.y * k.z,	c + t * k.z * k.z,			0.0f,
		0.0f,						0.0f,						0.0f,						1.0f
	};
}

Matrix4x4 GameMath::MakeYawMatrix(const Vector2& forwardXZ) {
	Vector2 forward = { 0.0f, 1.0f };
	TryNormalize(forwardXZ, forward);

	// 行0 = 右方向, 行1 = 上方向, 行2 = 前方向
	return {
		forward.y,	0.0f,	-forward.x,	0.0f,
		0.0f,		1.0f,	0.0f,		0.0f,
		forward.x,	0.0f,	forward.y,	0.0f,
		0.0f,		0.0f,	0.0f,		1.0f
	};
}

Matrix4x4 GameMath::MakeWorldMatrix(float uniformScale, const Matrix4x4& rotation, const Vector3& position) {
	Matrix4x4 result = Matrix4x4::Identity();
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = rotation.m[i][j] * uniformScale;
		}
	}
	result.m[3][0] = position.x;
	result.m[3][1] = position.y;
	result.m[3][2] = position.z;
	return result;
}

Matrix4x4 GameMath::Orthonormalize(const Matrix4x4& rotation) {
	Vector3 row0 = { rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] };
	Vector3 row1 = { rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] };

	if (!TryNormalize(row0, row0)) {
		return Matrix4x4::Identity();
	}
	row1 = row1 - row0 * Dot(row0, row1);
	if (!TryNormalize(row1, row1)) {
		return Matrix4x4::Identity();
	}
	Vector3 row2 = Cross(row0, row1);

	return {
		row0.x, row0.y, row0.z, 0.0f,
		row1.x, row1.y, row1.z, 0.0f,
		row2.x, row2.y, row2.z, 0.0f,
		0.0f,	0.0f,	0.0f,	1.0f
	};
}

bool GameMath::TryTransformPoint(const Vector3& p, const Matrix4x4& m, Vector3& out) {
	float x = p.x * m.m[0][0] + p.y * m.m[1][0] + p.z * m.m[2][0] + m.m[3][0];
	float y = p.x * m.m[0][1] + p.y * m.m[1][1] + p.z * m.m[2][1] + m.m[3][1];
	float z = p.x * m.m[0][2] + p.y * m.m[1][2] + p.z * m.m[2][2] + m.m[3][2];
	float w = p.x * m.m[0][3] + p.y * m.m[1][3] + p.z * m.m[2][3] + m.m[3][3];

	if (!std::isfinite(w) || std::fabs(w) <= 1.0e-6f) {
		return false;
	}

	out = { x / w, y / w, z / w };
	return std::isfinite(out.x) && std::isfinite(out.y) && std::isfinite(out.z);
}
