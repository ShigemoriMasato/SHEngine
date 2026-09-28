#pragma once
struct Vector2;
struct Vector3;
struct Vector4;

struct Matrix3x3;
struct Matrix4x4;

struct Quaternion;

Vector2 operator+(const Vector2& a, const Vector2& b);
Vector2 operator-(const Vector2& a, const Vector2& b);
Vector2 operator*(const Vector2& a, const Vector2& b);
Vector2 operator*(const Vector2& a, float b);
Vector2 operator*(float a, const Vector2& b);
Vector2 operator/(const Vector2& a, const Vector2& b);
Vector2 operator/(const Vector2& a, float b);
Vector2 operator+=(Vector2& a, const Vector2& b);
Vector2 operator-=(Vector2& a, const Vector2& b);
Vector2 operator*=(Vector2& a, const Vector2& b);
Vector2 operator*=(Vector2& a, float b);
Vector2 operator/=(Vector2& a, const Vector2& b);
Vector2 operator/=(Vector2& a, float b);
bool operator==(const Vector2& a, const Vector2& b);
bool operator!=(const Vector2& a, const Vector2& b);
Vector2 operator-(const Vector2& a);

Vector3 operator+(const Vector3& a, const Vector3& b);
Vector3 operator-(const Vector3& a, const Vector3& b);
Vector3 operator-(const Vector3& a);
Vector3 operator*(const Vector3& a, const Vector3& b);
Vector3 operator*(const Vector3& a, float b);
Vector3 operator*(float a, const Vector3& b);
Vector3 operator/(const Vector3& a, const Vector3& b);
Vector3 operator/(const Vector3& a, float b);
Vector3 operator+=(Vector3& a, const Vector3& b);
Vector3 operator-=(Vector3& a, const Vector3& b);
Vector3 operator*=(Vector3& a, const Vector3& b);
Vector3 operator*=(Vector3& a, float b);
Vector3 operator/=(Vector3& a, const Vector3& b);
Vector3 operator/=(Vector3& a, float b);
bool operator==(const Vector3& a, const Vector3& b);
bool operator!=(const Vector3& a, const Vector3& b);

Vector4 operator+(const Vector4& a, const Vector4& b);
Vector4 operator-(const Vector4& a, const Vector4& b);
Vector4 operator*(const Vector4& a, const Vector4& b);
Vector4 operator*(const Vector4& a, float b);
Vector4 operator*(float a, const Vector4& b);
Vector4 operator/(const Vector4& a, const Vector4& b);
Vector4 operator/(const Vector4& a, float b);
Vector4 operator+=(Vector4& a, const Vector4& b);
Vector4 operator-=(Vector4& a, const Vector4& b);
Vector4 operator*=(Vector4& a, const Vector4& b);
Vector4 operator*=(Vector4& a, float b);
Vector4 operator/=(Vector4& a, const Vector4& b);
Vector4 operator/=(Vector4& a, float b);
bool operator==(const Vector4& a, const Vector4& b);
bool operator!=(const Vector4& a, const Vector4& b);

Matrix3x3 operator+(const Matrix3x3& a, const Matrix3x3& b);
Matrix3x3 operator-(const Matrix3x3& a, const Matrix3x3& b);
Matrix3x3 operator*(const Matrix3x3& a, const Matrix3x3& b);
Vector2 operator*(const Matrix3x3& a, const Vector2& b);
Vector2 operator*(const Vector2& a, const Matrix3x3& b);
Matrix3x3 operator/(const Matrix3x3& a, const Matrix3x3& b);
Matrix3x3 operator+=(Matrix3x3& a, const Matrix3x3& b);
Matrix3x3 operator-=(Matrix3x3& a, const Matrix3x3& b);
Matrix3x3 operator*=(Matrix3x3& a, const Matrix3x3& b);
bool operator==(const Matrix3x3& a, const Matrix3x3& b);
bool operator!=(const Matrix3x3& a, const Matrix3x3& b);

Matrix4x4 operator+(const Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator-(const Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b);
Vector3 operator*(const Matrix4x4& a, const Vector3& b);
Vector3 operator*(const Vector3& a, const Matrix4x4& b);
Matrix4x4 operator/(const Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator+=(Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator-=(Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator*=(Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 operator/=(Matrix4x4& a, const Matrix4x4& b);
bool operator==(const Matrix4x4& a, const Matrix4x4& b);
bool operator!=(const Matrix4x4& a, const Matrix4x4& b);

Quaternion operator+(const Quaternion& a, const Quaternion& b);
Quaternion operator-(const Quaternion& a, const Quaternion& b);
Quaternion operator*(const Quaternion& a, const float b);
