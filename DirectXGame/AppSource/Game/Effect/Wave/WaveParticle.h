#pragma once
#include <Assets/Texture/TextureManager.h>
#include <Compute/ComputeObject.h>
#include <Render/Renderer.h>
#include <Game/Effect/Pool/ParticlePool.h>
#include <Tool/Binary/BinaryManager.h>
#include <random>

struct WaveData {
	Vector3 position{};
	float speed = 2.0f;
	Vector3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;
	float lifetime = 100000.0f;				//勝手に起動しないように大きな数字にする
	float decayRate = 0.1f;
	float maxlifetime = 3.0f;
	float thickness = 1.0f;
};

class WaveParticle {
public:

	~WaveParticle();

	void Initialize(SHEngine::TextureManager* textureManager, const Pool& pool, const uint32_t id);
	void Update(CmdObj* compute, float deltaTime);

	void DrawImGui();

	void AddWave(const WaveData& waveData);

private:

	void Load();
	void Save();

	void CopyConfig(float deltaTime);

	SHEngine::TextureManager* texturemanager_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	std::vector<std::unique_ptr<SHEngine::ComputeObject>> emitter_;
	std::vector<std::unique_ptr<SHEngine::ComputeObject>> update_;

	std::vector<SHEngine::GPUBuffer*> updateBuffer_;
	std::vector<SHEngine::GPUBuffer*> emitBuffer_;
	SHEngine::GPUBuffer* waveBuffer_ = nullptr;

	BinaryManager binaryManager_{};
	std::string fileName_ = "WaveParticleConfig.bin";

	int maxParticleNum_ = 0;

	struct EmitData {
		Vector3 fieldSize;
		float speed;

		Vector3 color;
		float lifeTime;

		int emitNum;
		uint32_t seed;
		uint32_t textureID;
		uint32_t executeOffset;
	}emitData_;
	struct UpdateData {
		Matrix4x4 parentMatrix;
		float lifeTime;
		Vector3 fieldSize;
		uint32_t executeOffset;
	}updateData_;

	struct Config {
		float speed = 4.0f;
		float lifeTime = 1.0f;
		int emitNum = 3;
		Vector3 fieldSize = { 10.0f, 0.0f, 10.0f };
		Vector3 position;
		Vector3 rotate;
		Vector3 scale = { 1.0f, 1.0f, 1.0f };
		Vector3 color = { 1.0f, 1.0f, 1.0f };
		int currentTextureID = 1;
	}config_;

	std::vector<WaveData> waves_;
	constexpr static inline int kMaxWaveNum_ = 16;

	std::mt19937 randomEngine_{ std::random_device{}() };
	std::uniform_int_distribution<uint32_t> randomDist_{ 0, UINT32_MAX };

	std::vector<std::string> texturePahts_;
};
