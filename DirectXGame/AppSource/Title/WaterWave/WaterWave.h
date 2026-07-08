#pragma once
#include <Render/RenderObject.h>

class WaterWave {
public:

	WaterWave() = default;
	~WaterWave();

	struct Wave {
		Vector2 direction;
		float amplitude;
		float wavelength;
		float speed;
		Vector3 pad;
	};

	void Initialize(const SHEngine::DrawData& drawData, Camera* camera);
	void Update(float deltaTime);
	void DrawImGui();
	void Draw(DCC* dcc);

	float time_ = 0.0f;

	std::vector<Wave> waves_{};

private:

	void Save();
	void Load();

	struct UniqueData {
		Matrix4x4 world;
		Matrix4x4 vp;
		Vector3 cameraPos;
		float time;
		int waveCount = 8;
		Vector3 pad;
		Wave waves[8];
	};

	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	std::unique_ptr<SHEngine::RenderObject> renderObject_ = nullptr;
	UniqueData uniqueData_{};
	DirectionalLight light_{};
	Vector4 baseColor_ = { 0.0f, 0.3f, 0.5f, 1.0f };

	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 position = { 0.0f, 0.0f, 0.0f };

	Camera* camera_ = nullptr;

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;

};

