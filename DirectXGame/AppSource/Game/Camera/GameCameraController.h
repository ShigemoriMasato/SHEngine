#pragma once
#include <Camera/Camera.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief ゲームカメラの調整値。反映タイミング: 次プレイから
struct CameraConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "GameCameraConfig.bin";

	float distance = 20.0f;			// 注視点(プレイヤー)からカメラまでの距離(ワールド単位, 1～500)
	float pitch = 1.1f;				// 見下ろし角(ラジアン, 水平0～真下π/2。0.1～1.5)
	float followSharpness = 6.0f;	// 追従の鋭さ(1/秒, 0.1～100)。大きいほど速く追いつく
	float fovY = 0.8f;				// 縦視野角(ラジアン, 0.1～2.5)
	float nearClip = 0.1f;			// ニアクリップ(ワールド単位, 0.01～10)
	float farClip = 1000.0f;		// ファークリップ(ワールド単位, 10～100000)

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void Validate();
};

/// @brief プレイヤーを画面中央に保ち、一定距離を保ちながら補間で滑らかに追従するカメラ。
/// カメラ行列はAimControllerとRendererへだけ渡し、他のゲーム更新処理には渡さない。
class GameCameraController {
public:

	/// @param screenSize 描画先の解像度(アスペクト比に使う)
	/// @param target 初期の注視点(ワールド)。最初は補間せず即座に合わせる
	void Initialize(const Vector2& screenSize, const Vector3& target);

	/// @brief (3) 注視点を補間して追従し、行列を更新する
	void Update(float deltaTime, const Vector3& target);

	const Camera* GetCamera() const { return &camera_; }
	Matrix4x4 GetViewMatrix() const { return camera_.GetViewMatrix(); }
	Matrix4x4 GetViewProjectionMatrix() const { return camera_.GetVPMatrix(); }

	void SaveConfig() const;

private:

	void ApplyMatrix();

	CameraConfig config_{};
	Camera camera_;
	Vector3 focus_ = {};	// 現在の注視点(補間後)
};
