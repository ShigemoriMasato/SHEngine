#pragma once
#include <SHEngine.h>
#include <Render/Renderer.h>
#include <random>
#include <numbers>

class HitEffect {
public:

	void Initialize(SHEngine::Engine* engine);
	void Update(float deltaTime, const Matrix4x4& vpMat);
	void Draw(CmdObj* cmdObj);

private:

	void Spawn(const Matrix4x4& vpMat);

	std::unique_ptr<SHEngine::BufferContainer> container_;
	std::unique_ptr<SHEngine::Renderer> renderer_;

	SHEngine::GPUBuffer* wvp_;
	SHEngine::GPUBuffer* colorBuffer_;

	const uint32_t spawnNum_ = 16;

	std::mt19937 randEngine_{ std::random_device{}() };
	std::uniform_real_distribution<float> randDist_{ -std::numbers::pi_v<float>, std::numbers::pi_v<float> };

	Matrix4x4 translate_;
	std::vector<Matrix4x4> worlds_;

	float timer_ = 0.0f;
	const float life_ = 1.0f;
	const float spawnInterval_ = life_ + 1.0f;
};

