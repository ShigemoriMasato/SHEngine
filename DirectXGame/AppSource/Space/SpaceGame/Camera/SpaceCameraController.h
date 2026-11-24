#pragma once
#include <Camera/Camera.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;
struct SpaceInput;

/// @brief SpaceGameカメラの調整値
struct SpaceCameraConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "SpaceCameraConfig.bin";

	float distance = 10.0f;			// 注視点からカメラまでの距離(ワールド単位, 1～500)。即時反映
	float mouseSensitivity = 0.003f;// マウス1ピクセルあたりの回転量(ラジアン, 0～0.1)。即時反映
	float stickSensitivity = 3.0f;	// 右スティック最大傾き時の回転速度(ラジアン/秒, 0～20)。即時反映
	float minPitch = -1.4f;			// 見上げの限界(ラジアン, -1.55～0)。即時反映
	float maxPitch = 1.4f;			// 見下ろしの限界(ラジアン, 0～1.55)。即時反映
	float followSharpness = 10.0f;	// 注視点の追従の鋭さ(1/秒, 0.1～100)。即時反映
	float fovY = 0.8f;				// 縦視野角(ラジアン, 0.1～2.5)。即時反映
	float nearClip = 0.1f;			// ニアクリップ(ワールド単位, 0.01～10)。即時反映
	float farClip = 1000.0f;		// ファークリップ(ワールド単位, 10～100000)。即時反映
	bool invertY = false;			// 上下の視点操作を反転する。即時反映
	bool mouseLookOnRightClick = false;	// trueなら右クリック中だけマウスで視点操作する。即時反映

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void DrawImGui();
	void Validate();
};

/// @brief 注視点(プレイヤー)の周りを回る三人称カメラ。マウス／右スティックでヨー・ピッチを操作する
class SpaceCameraController {
public:

	/// @param screenSize 描画先の解像度(アスペクト比に使う)
	/// @param target 初期の注視点。最初は補間せず即座に合わせる
	void Initialize(const Vector2& screenSize, const Vector3& target);

	/// @brief 視点入力で回転し、注視点を補間して追従する
	void Update(float deltaTime, const Vector3& target, const SpaceInput& input);

	void DrawImGui();
	void SaveConfig() const;

	const Camera* GetCamera() const { return &camera_; }
	const SpaceCameraConfig& GetConfig() const { return config_; }

	/// @brief 水平方向の向き(ラジアン)。0で+Z、正でX+側へ回る
	float GetYaw() const { return yaw_; }
	float GetPitch() const { return pitch_; }

private:

	void ApplyProjection();
	void ApplyMatrix();

	SpaceCameraConfig config_{};
	Camera camera_;

	Vector2 screenSize_ = { 1280.0f, 720.0f };

	Vector3 focus_ = {};	// 現在の注視点(補間後)
	float yaw_ = 0.0f;
	float pitch_ = 0.3f;	// 正で見下ろし
};
