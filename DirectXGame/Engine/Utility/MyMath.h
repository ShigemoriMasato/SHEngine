#pragma once
#include <cstdint>
#include <vector>
#include "Quaternion.h"
#include "Matrix.h"
#include "Vector.h"

namespace MyMath {

	float Repeat(float value, float max, float min = 0.0f);

	Vector3 ConvertVector(const Vector4& v);
	Vector4 ConvertVector(const Vector3& v, float w);

	float cot(float radian);

	Vector3 Normalize(Vector3 vec);

	Vector3 cross(const Vector3& a, const Vector3& b);
	float dot(const Vector3& a, const Vector3& b);
	float Length(const Vector3& vec);

	std::vector<Vector3> GetSplinePoints(const std::vector<Vector3>& controlPoints, int segment);
	std::vector<Vector2> GetSplinePoints(const std::vector<Vector2>& controlPoints, int segment);
	std::vector<Vector3> GetCatmullPoints(const std::vector<Vector3>& points, int segmentsPerCurve);
	Vector3 GetCatmullPoint(const std::vector<Vector3>& points, float t);
	Vector2 GetSplinePoint(const std::vector<Vector2>& controlPoints, float t);
	float CalculateAngle(const Vector3& a, const Vector3& b);

}
