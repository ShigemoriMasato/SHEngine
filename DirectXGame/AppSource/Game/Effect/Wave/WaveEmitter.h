#pragma once
#include <Game/Effect/Base/IEmitter.h>
#include <Tool/Binary/BinaryManager.h>

//平面である代わりに波紋を出せる
class WaveEmitter : public IEmitter {
public:

	struct Config {
		Vector3 color;
		float speed;
		Vector3 fieldSize;
		int emitNum;
		uint32_t textureID;
		float colorIntensity;
		float lifeTime;

		Vector3 scale;
		Vector3 rotate;
		Vector3 position;

		void DrawImGui();
		void Save(BinaryManager& binaryManager);
		void Load(BinaryManager& binaryManager);
	};

	struct WaveData {
		Vector3 position{};
		float speed = 48.0f;
		Vector3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
		float lifetime = 0.0f;
		float decayRate = 0.1f;
		float maxlifetime = 0.0f;
		float thickness = 4.0f;

		bool DrawImGui();
		void Save(BinaryManager& binaryManager);
		void Load(BinaryManager& binaryManager);
	};

public:

	WaveEmitter(uint32_t kMaxParticleNum = 1000000) : kMaxParticleNum_(kMaxParticleNum) {};
	~WaveEmitter();

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* ccc, float deltaTime) override;

	void AddWave(const WaveData& waveData);
	void AddWave(const std::vector<WaveData>& waves);
	void SetWave(const std::vector<WaveData>& waves) { waves_ = waves; }

	void SetConfig(const Config& config) { config_ = config; }

	void DrawImGui();

private:

	void CopyConfig(float deltaTime);

	const uint32_t kMaxParticleNum_;

	SHEngine::TextureManager* texturemanager_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> emit_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	SHEngine::GPUBuffer* waveBuffer_ = nullptr;

	SHEngine::GPUBuffer* lifeTime_ = nullptr;
	SHEngine::GPUBuffer* updateData_ = nullptr;
	SHEngine::GPUBuffer* emitData_ = nullptr;

	Config config_{};

	struct EmitData {
		Vector3 fieldSize;
		float speed;
		Vector3 color;
		int emitNum;
		uint32_t seed;
		uint32_t textureID;
	} emitValue_{};
	struct UpdateData {
		Matrix4x4 parentMatrix;
		Vector3 fieldSize;
		float colorIntensity;
	} updateValue_{};

	std::vector<WaveData> waves_{};
	constexpr static inline int kMaxWaveNum_ = 16;

	std::vector<std::string> texturePahts_;
};
