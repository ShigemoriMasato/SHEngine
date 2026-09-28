#pragma once
#include "Operator.h"
#include "Vector.h"
#include "Matrix.h"

struct Quaternion {
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

	// 単位クオータニオン生成（回転なし）
	static Quaternion Identity() {
		return { 0, 0, 0, 1 };
	}

	// 軸と角度から生成
	static Quaternion FromAxisAngle(const Vector3& axis, float angle) {
		float half = angle * 0.5f;
		float s = sin(half);
		return {
			axis.x * s,
			axis.y * s,
			axis.z * s,
			cos(half)
		};
	}

	// 正規化
	Quaternion Normalize() const {
		float len = sqrt(x * x + y * y + z * z + w * w);
		if (len == 0.0f) {
			return { 0, 0, 0, 1 };
		}
		return { x / len, y / len, z / len, w / len };
	}

	Matrix4x4 ToMatrix() const {
		// 正規化
		Quaternion a = Normalize();

		float x2 = a.x * a.x;
		float y2 = a.y * a.y;
		float z2 = a.z * a.z;
		float w2 = a.w * a.w;

		return {
			w2 + x2 - y2 - z2, 2 * (a.x * a.y + a.w * a.z), 2 * (a.x * a.z - a.w * a.y), 0,
			2 * (a.x * a.y - a.w * a.z), w2 - x2 + y2 - z2, 2 * (a.y * a.z + a.w * a.x), 0,
			2 * (a.x * a.z + a.w * a.y), 2 * (a.y * a.z - a.w * a.x), w2 - x2 - y2 + z2, 0,
			0, 0, 0, 1
		};
	}
};
