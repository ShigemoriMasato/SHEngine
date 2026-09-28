#pragma once
#include "Vector.h"
#include "Matrix.h"

namespace Matrix {

	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	Matrix4x4 MakeTranslationMatrix(const Vector3& pos);

	Matrix4x4 MakeRotationMatrix(Vector3 angle);
	Matrix4x4 MakeRotationXMatrix(float angle);
	Matrix4x4 MakeRotationYMatrix(float angle);
	Matrix4x4 MakeRotationZMatrix(float angle);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& position);


	Matrix3x3 MakeTranslationMatrix(const Vector2& pos);
	Matrix3x3 MakeRotationMatrix(float angle);
	Matrix3x3 MakeScaleMatrix(const Vector2& scale);
	Matrix3x3 MakeAffineMatrix(const Vector2& scale, const float angle, const Vector2 translation);

	Matrix3x3 TransMatrix(const Matrix3x3& m);
	Matrix4x4 TransMatrix(const Matrix4x4& m);


	Matrix4x4 MakeUVMatrix(Vector2 scale, float rotate, Vector2 position);

	Matrix4x4 InverseMatrix(const Matrix4x4& matrix);
	Matrix4x4 Transpose(const Matrix4x4& matrix);

	Matrix4x4 InverseTranspose(const Matrix4x4& m);
}
