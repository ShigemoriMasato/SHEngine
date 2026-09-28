#include "../Operator.h"
#include "../Matrix.h"
#include "../Vector.h"
#include "../Quaternion.h"
#include <cmath>
#include <cassert>

namespace {
	bool ZeroChecker(float* value, int size) {
		bool isNotZero = true;

		for (int i = 0; i < size; ++i) {
			if (std::fabsf(value[i]) <= 1e-6) {
				isNotZero = false;
				break;
			}
		}

		return isNotZero;
	}
}

Vector2 operator+(const Vector2& a, const Vector2& b) {
	return Vector2(a.x + b.x, a.y + b.y);
}

Vector2 operator-(const Vector2& a, const Vector2& b) {
	return Vector2(a.x - b.x, a.y - b.y);
}

Vector2 operator*(const Vector2& a, const Vector2& b) {
	return Vector2(a.x * b.x, a.y * b.y);
}

Vector2 operator*(const Vector2& a, float b) {
	return Vector2(a.x * b, a.y * b);
}

Vector2 operator*(float a, const Vector2& b) {
	return Vector2(a * b.x, a * b.y);
}

Vector2 operator/(const Vector2& a, const Vector2& b) {
	if (!ZeroChecker((float*)&b, 2)) {
		assert(false && "Division by zero in Vector2 operator/");
	}
	return Vector2(a.x / b.x, a.y / b.y);
}

Vector2 operator/(const Vector2& a, float b) {
	if (std::fabsf(b) <= 1e-6) {
		assert(false && "Division by zero in Vector2 operator/");
	}
	return Vector2(a.x / b, a.y / b);
}

Vector2 operator+=(Vector2& a, const Vector2& b) {
	a.x += b.x;
	a.y += b.y;
	return a;
}

Vector2 operator-=(Vector2& a, const Vector2& b) {
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

Vector2 operator*=(Vector2& a, const Vector2& b) {
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

Vector2 operator*=(Vector2& a, float b) {
	a.x *= b;
	a.y *= b;
	return a;
}

Vector2 operator/=(Vector2& a, const Vector2& b) {
	if (!ZeroChecker((float*)&b, 2)) {
		assert(false && "Division by zero in Vector2 operator/=");
	}
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

Vector2 operator/=(Vector2& a, float b) {
	if (std::fabsf(b) <= 1e-6) {
		assert(false && "Division by zero in Vector2 operator/=");
	}
	a.x /= b;
	a.y /= b;
	return a;
}

bool operator==(const Vector2& a, const Vector2& b) {
	return a.x == b.x && a.y == b.y;
}

bool operator!=(const Vector2& a, const Vector2& b) {
	return !(a == b);
}

Vector2 operator-(const Vector2& a) {
	return Vector2(-a.x, -a.y);
}

Vector3 operator+(const Vector3& a, const Vector3& b) {
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 operator-(const Vector3& a, const Vector3& b) {
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 operator-(const Vector3& a) {
	return Vector3(-a.x, -a.y, -a.z);
}

Vector3 operator*(const Vector3& a, const Vector3& b) {
	return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vector3 operator*(const Vector3& a, float b) {
	return Vector3(a.x * b, a.y * b, a.z * b);
}

Vector3 operator*(float a, const Vector3& b) {
	return Vector3(a * b.x, a * b.y, a * b.z);
}

Vector3 operator/(const Vector3& a, const Vector3& b) {
	if (!ZeroChecker((float*)&b, 3)) {
		assert(false && "Division by zero in Vector3 operator/");
	}
	return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
}

Vector3 operator/(const Vector3& a, float b) {
	if (!ZeroChecker(&b, 1)) {
		assert(false && "Division by zero in Vector3 operator/");
	}
	return Vector3(a.x / b, a.y / b, a.z / b);
}

Vector3 operator+=(Vector3& a, const Vector3& b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

Vector3 operator-=(Vector3& a, const Vector3& b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

Vector3 operator*=(Vector3& a, const Vector3& b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

Vector3 operator*=(Vector3& a, float b) {
	a.x *= b;
	a.y *= b;
	a.z *= b;
	return a;
}

Vector3 operator/=(Vector3& a, const Vector3& b) {
	if (!ZeroChecker((float*)&b, 3)) {
		assert(false && "Division by zero in Vector3 operator/=");
	}
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

Vector3 operator/=(Vector3& a, float b) {
	if (!ZeroChecker(&b, 1)) {
		assert(false && "Division by zero in Vector3 operator/=");
	}
	a.x /= b;
	a.y /= b;
	a.z /= b;
	return a;
}

bool operator==(const Vector3& a, const Vector3& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const Vector3& a, const Vector3& b) {
	return !(a == b);
}

Vector4 operator+(const Vector4& a, const Vector4& b) {
	return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vector4 operator-(const Vector4& a, const Vector4& b) {
	return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vector4 operator*(const Vector4& a, const Vector4& b) {
	return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

Vector4 operator*(const Vector4& a, float b) {
	return Vector4(a.x * b, a.y * b, a.z * b, a.w * b);
}

Vector4 operator*(float a, const Vector4& b) {
	return Vector4(a * b.x, a * b.y, a * b.z, a * b.w);
}

Vector4 operator/(const Vector4& a, const Vector4& b) {
	if (!ZeroChecker((float*)&b, 4)) {
		assert(false && "Division by zero in Vector4 operator/");
	}
	return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

Vector4 operator/(const Vector4& a, float b) {
	if (!ZeroChecker(&b, 1)) {
		assert(false && "Division by zero in Vector4 operator/");
	}
	return Vector4(a.x / b, a.y / b, a.z / b, a.w / b);
}

Vector4 operator+=(Vector4& a, const Vector4& b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

Vector4 operator-=(Vector4& a, const Vector4& b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}

Vector4 operator*=(Vector4& a, const Vector4& b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
	return a;
}

Vector4 operator*=(Vector4& a, float b) {
	a.x *= b;
	a.y *= b;
	a.z *= b;
	a.w *= b;
	return a;
}

Vector4 operator/=(Vector4& a, const Vector4& b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
	return a;
}

Vector4 operator/=(Vector4& a, float b) {
	a.x /= b;
	a.y /= b;
	a.z /= b;
	a.w /= b;
	return a;
}

bool operator==(const Vector4& a, const Vector4& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

bool operator!=(const Vector4& a, const Vector4& b) {
	return !(a == b);
}

Matrix3x3 operator+(const Matrix3x3& a, const Matrix3x3& b) {
	Matrix3x3 result;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = a.m[i][j] + b.m[i][j];
		}
	}
	return result;
}

Matrix3x3 operator-(const Matrix3x3& a, const Matrix3x3& b) {
	Matrix3x3 result;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result.m[i][j] = a.m[j][i] - b.m[i][j];
		}
	}
	return result;
}

Matrix3x3 operator*(const Matrix3x3& a, const Matrix3x3& b) {
	Matrix3x3 result = {};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			for (int k = 0; k < 3; ++k) {
				result.m[j][i] += a.m[k][i] * b.m[j][k];
			}
		}
	}
	return result;
}

Vector2 operator*(const Matrix3x3& m, const Vector2& v) {
	Vector2 ans;
	ans.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2];
	ans.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2];
	float w = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2];
	if (std::fabsf(w) <= 1e-6) {
		assert(false && "Division by zero in Vector2 operator* with Matrix3x3");
	}
	ans.x /= w;
	ans.y /= w;
	return ans;
}

Vector2 operator*(const Vector2& a, const Matrix3x3& b) {
	return b * a;
}

Matrix3x3 operator/(const Matrix3x3& a, const Matrix3x3& b) {
	if (ZeroChecker((float*)&b, 9)) {
		assert(false && "Division by zero in Matrix3x3 operator/");
	}
	return a * b.Inverse();
}

Matrix3x3 operator+=(Matrix3x3& a, const Matrix3x3& b) {
	a = a + b;
	return a;
}

Matrix3x3 operator-=(Matrix3x3& a, const Matrix3x3& b) {
	a = a - b;
	return a;
}

Matrix3x3 operator*=(Matrix3x3& a, const Matrix3x3& b) {
	a = a * b;
	return a;
}

Matrix3x3 operator/=(Matrix3x3& a, const Matrix3x3& b) {
	if (ZeroChecker((float*)&b, 9)) {
		assert(false && "Division by zero in Matrix3x3 operator/=");
	}
	a = a / b;
	return a;
}

bool operator==(const Matrix3x3& a, const Matrix3x3& b) {
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (a.m[i][j] != b.m[i][j]) {
				return false;
			}
		}
	}
	return true;
}

bool operator!=(const Matrix3x3& a, const Matrix3x3& b) {
	return !(a == b);
}

Matrix4x4 operator+(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = a.m[i][j] + b.m[i][j];
		}
	}
	return result;
}

Matrix4x4 operator-(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = a.m[i][j] - b.m[i][j];
		}
	}
	return result;
}

Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += a.m[i][k] * b.m[k][j];
			}
		}
	}
	return result;
}

Vector3 operator*(const Matrix4x4& mat, const Vector3& vec) {
	Vector3 ans;
	ans.x = vec.x * mat.m[0][0] + vec.y * mat.m[1][0] + vec.z * mat.m[2][0] + mat.m[3][0];
	ans.y = vec.x * mat.m[0][1] + vec.y * mat.m[1][1] + vec.z * mat.m[2][1] + mat.m[3][1];
	ans.z = vec.x * mat.m[0][2] + vec.y * mat.m[1][2] + vec.z * mat.m[2][2] + mat.m[3][2];
	float w = vec.x * mat.m[0][3] + vec.y * mat.m[1][3] + vec.z * mat.m[2][3] + mat.m[3][3];
	assert(w != 0.0f);
	ans.x /= w;
	ans.y /= w;
	ans.z /= w;
	return ans;
}

Vector3 operator*(const Vector3& a, const Matrix4x4& b) {
	return b * a;
}

Matrix4x4 operator/(const Matrix4x4& a, const Matrix4x4& b) {
	if (ZeroChecker((float*)&b, 16)) {
		assert(false && "Division by zero in Matrix4x4 operator/");
	}
	return a * b.Inverse();
}

Matrix4x4 operator+=(Matrix4x4& a, const Matrix4x4& b) {
	a = a + b;
	return a;
}

Matrix4x4 operator-=(Matrix4x4& a, const Matrix4x4& b) {
	a = a - b;
	return a;
}

Matrix4x4 operator*=(Matrix4x4& a, const Matrix4x4& b) {
	a = a * b;
	return a;
}

Matrix4x4 operator/=(Matrix4x4& a, const Matrix4x4& b) {
	a = a * b.Inverse();
	return a;
}

bool operator==(const Matrix4x4& a, const Matrix4x4& b) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (a.m[i][j] != b.m[i][j]) {
				return false;
			}
		}
	}
	return true;
}

bool operator!=(const Matrix4x4& a, const Matrix4x4& b) {
	return !(a == b);
}

Quaternion operator+(const Quaternion& a, const Quaternion& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

Quaternion operator-(const Quaternion& a, const Quaternion& b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

Quaternion operator*(const Quaternion& a, const float b) {
	return { a.x * b, a.y * b, a.z * b, a.w * b };
}
