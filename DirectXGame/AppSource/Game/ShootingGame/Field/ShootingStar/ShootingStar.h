#pragma once
#include <Render/Renderer.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <random>

class ShootingStar {
public:

	ShootingStar(SHEngine::BufferContainer* container, const Mesh& mesh, ImGuiWindowManager* imguiManager);

	void Initialize(uint32_t textureSize, Camera* camera);
	void Update(float deltaTime);
	void Draw(DCC* dcc);

	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);

private:

	SHEngine::GPUBuffer* colorBuffer_ = nullptr;
	SHEngine::GPUBuffer* scaleBuffer_ = nullptr;
	SHEngine::GPUBuffer* positionBuffer_ = nullptr;

	std::unique_ptr<SHEngine::Renderer> renderer_ = nullptr;
	std::vector<Vector2> positions_ = {};

	float timer_ = 0.0f;

	float frequency_ = 0.5f;
	uint32_t emitNum_ = 2;
	Vector2 velocity_ = { 0.0f, 0.0f };
	float scale_ = 24.0f;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::mt19937 seed_ = std::mt19937(std::random_device()());
	std::uniform_real_distribution<float> dist_ = std::uniform_real_distribution<float>(0.0f, 1.0f);

	const uint32_t starNum_ = 256;
	uint32_t textureSize_ = 1024;
};

