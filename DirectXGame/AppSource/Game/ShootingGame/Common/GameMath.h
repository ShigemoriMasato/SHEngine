#pragma once
#include <Utility/Vector.h>
#include <Utility/Matrix.h>

// ゲームロジック用の計算ヘルパー。
// ゲームはXZ平面上で行うため、ロジック側の位置・速度・方向は Vector2(x = ワールドX, y = ワールドZ) で扱い、
// 描画スナップショットを作る段階でだけ Vector3(Y = 高さ) へ変換する。
// 座標系はエンジン既存の規約(左手系・Y上・行ベクトル × 行列)に合わせる。
namespace GameMath {

	// 速度をゼロとみなす閾値(ワールド単位/秒)。減衰後にこれ未満なら0へ丸める
	constexpr float kStopSpeed = 1.0e-3f;
	// 方向ベクトルとして正規化を許可する最小の長さ
	constexpr float kDirectionEpsilon = 1.0e-5f;

	float Dot(const Vector2& a, const Vector2& b);
	float Dot(const Vector3& a, const Vector3& b);
	Vector3 Cross(const Vector3& a, const Vector3& b);

	/// @brief ゼロ・非有限ベクトルを除外して正規化する。失敗時はoutを変更せずfalse
	bool TryNormalize(const Vector2& v, Vector2& out);
	bool TryNormalize(const Vector3& v, Vector3& out);

	/// @brief ベクトルの長さをmaxLength以下に制限する
	Vector2 ClampLength(const Vector2& v, float maxLength);

	/// @brief 1秒あたりの残存率dampingを時間基準で適用する(velocity * damping^deltaTime)
	Vector2 ApplyDamping(const Vector2& velocity, float damping, float deltaTime);

	/// @brief XZ平面上の座標を高さ付きのワールド座標へ変換する
	Vector3 ToWorld(const Vector2& xz, float height);

	bool IsFinite(float value);
	bool IsFinite(const Vector2& v);
	bool IsFinite(const Vector4& v);

	/// @brief 単位軸周りの回転行列(行ベクトル規約。d/dθ(x * M) = axis × x となる向き)
	Matrix4x4 MakeAxisAngleMatrix(const Vector3& unitAxis, float angle);
	/// @brief モデルの+ZをXZ平面上のforwardへ向けるY軸回転行列
	Matrix4x4 MakeYawMatrix(const Vector2& forwardXZ);
	/// @brief 一様スケール * 回転 * 平行移動 のワールド行列
	Matrix4x4 MakeWorldMatrix(float uniformScale, const Matrix4x4& rotation, const Vector3& position);
	/// @brief 累積誤差で崩れた回転行列を正規直交化する
	Matrix4x4 Orthonormalize(const Matrix4x4& rotation);
	/// @brief w除算付きで点を変換する。wが0に近い・結果が非有限の場合false
	bool TryTransformPoint(const Vector3& point, const Matrix4x4& matrix, Vector3& out);
}
