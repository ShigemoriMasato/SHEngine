#pragma once
#include <Render/RenderObject.h>
#include <Tool/Binary/BinaryManager.h>

class Title {
public:

	~Title();

	void Initialize(SHEngine::DrawData& drawData, Camera* camera);
	void Update(float deltaTime);
	void Draw(DCC* cmdObj);
	void DrawImGui();

private:

	void Save();
	void Load();

	struct VSData {
		Matrix4x4 world;
		Matrix4x4 vp;
		uint32_t color;
		uint32_t outlineColor;
		uint32_t pad[2];
	} data;

	Camera* camera_ = nullptr;
	std::unique_ptr<SHEngine::RenderObject> renderObject_{};

	uint32_t lightColor_ = 0xffffffff;
	uint32_t darkColor_ = 0xff202020;
	uint32_t outlineColor_ = 0xffffffff;

	float time_ = 0.0f;
	float toggleTime_ = 0.0f;
	bool isLighting_ = false;

	Vector3 scale_ = { 1.0f, 1.0f, 1.0f };
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };

	std::unique_ptr<BinaryManager> binaryManager_ = nullptr;
};
