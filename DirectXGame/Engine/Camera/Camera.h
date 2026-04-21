#pragma once
#include <Utility/MatrixFactory.h>
#include <memory>

//初期値を画面サイズ1280*720と仮定して設定

/**
 * @struct PerspectiveFovDesc
 * @brief 透視投影の設定パラメータ
 */
struct PerspectiveFovDesc {
	/// @brief 設定をまとめて行う関数
	/// @param width 画面の幅
	/// @param height 画面の高さ
	/// @param fovY 視野角Y（ラジアン）
	/// @param nearClip 表示する一番近いZ値
	/// @param farClip 表示する一番遠いZ値
	void SetValue(float width = 1280, float height = 720, float fovY = 0.45f, float nearClip = 0.1f, float farClip = 1000.0f) {
		this->fovY = fovY;
		this->aspectRatio = width / height;
		this->nearClip = nearClip;
		this->farClip = farClip;
	}
	/// @brief 視野角Y（ラジアン）
	float fovY = 0.45f;
	/// @brief アスペクト比
	float aspectRatio = 1280.0f / 720.0f;
	/// @brief ニアクリップ面
	float nearClip = 0.1f;
	/// @brief ファークリップ面
	float farClip = 1000.0f;
};

/**
 * @struct OrthographicDesc
 * @brief 正射影投影の設定パラメータ
 */
struct OrthographicDesc {
	OrthographicDesc SetValue(float width = 1280, float height = 720, float zeroPointX = 0.0f, float zeroPointY = 0.0f, float nearClip = 0.0f, float farClip = 1000.0f) {
		this->left = -zeroPointX;
		this->top = -zeroPointY;
		this->right = -zeroPointX + width;
		this->bottom = -zeroPointY + height;
		this->nearClip = nearClip;
		this->farClip = farClip;
		return *this;
	}
	/// @brief 左端
	float left = -0.0f;
	/// @brief 上端
	float top = 0.0f;
	/// @brief 右端
	float right = 1280;
	/// @brief 下端
	float bottom = 720;
	/// @brief ニアクリップ面
	float nearClip = 0.0f;
	/// @brief ファークリップ面
	float farClip = 1000.0f;
};

/**
 * @class Camera
 * @brief 3Dカメラを管理するクラス
 * 
 * カメラの位置、回転、スケールを管理し、
 * 透視投影または正射影投影のView-Projection行列を生成する。
 */
class Camera {
public:

	Camera() = default;
	~Camera() = default;
	
	void SetProjectionMatrix(PerspectiveFovDesc desc);
	void SetProjectionMatrix(OrthographicDesc desc);

	//TransformとProjectionMatrixをかけて、VPMatrixを作成する
	virtual void MakeMatrix();
	virtual void DrawImGui();

	void SetTransform(Matrix4x4 mat);
	void SetScale(Vector3 scale) { scale_ = scale; }
	void SetRotation(Vector3 rotation) { rotation_ = rotation; }
	void SetPosition(Vector3 position) { position_ = position; }

	//VPMatrixを取得
	virtual Matrix4x4 GetVPMatrix() const;
	virtual Matrix4x4 GetBillboardMatrix() const;

	Matrix4x4 GetTransformMatrix() const { return transformMatrix_; }
	Matrix4x4 GetProjectionMatrix() const { return projectionMatrix_; }

protected:

	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
	Vector3 scale_ = { 1.0f, 1.0f, 1.0f };

	/// @brief 投影行列（ワールド行列にかけると投影変換される）
	Matrix4x4 projectionMatrix_ = Matrix4x4::Identity();
	/// @brief View-Projection行列（Viewport変換抜き）
	Matrix4x4 vpMatrix_ = Matrix4x4::Identity();

	/// @brief カメラ座標変換行列
	Matrix4x4 transformMatrix_ = Matrix4x4::Identity();

	/// @brief 投影行列が設定済みかどうか
	bool isSetMatrix = false;
};

