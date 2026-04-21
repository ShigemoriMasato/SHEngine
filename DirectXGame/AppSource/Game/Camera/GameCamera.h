#pragma once
#include <Camera/Camera.h>
#include <Tool/Binary/BinaryManager.h>

class GameCamera {
public:

	GameCamera() = default;
	~GameCamera();

	void Initialize();
	void Update(float deltaTime);
	void Draw();

	void Shake(float intensity, float duration);
	void Back(float distance, float speed);

	void DrawImGui();

	Camera* GetCamera() const { return camera_.get(); }

private:

	void Save();
	void Load();

	std::unique_ptr<Camera> camera_{};

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
	std::string saveFile_ = "GameCamera.sg";

	Vector3 position_{};
	Vector3 rotation_{};

	Vector3 factPos_{};
	Vector3 factRot_{};

	float shakeIntensity_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeTime_ = 0.0f;

	float beginningT_ = 1.0f;
	Vector3 beginningPos_{};
	Vector3 beginningRot_{};

private://ImGui
	bool beginningEdit_ = false;
};
