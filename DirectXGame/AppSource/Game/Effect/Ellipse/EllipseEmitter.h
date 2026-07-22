#pragma once
#include <Game/Effect/Base/IEmitter.h>

//一度だけトリガー方式でパーティクルを出力するエミッター
class EllipseEmitter : public IEmitter {
public:

	struct Config {
		Vector2 posA;
		Vector2 posB;
		float z;
		float radius;

		float lifeTime;
		float initialSpeed;

		Vector4 color;
		Transform transform;
		uint32_t emitCount;

		bool DrawImGui();

		void Save(BinaryManager& binaryManager) const;
		void Load(BinaryManager& binaryManager);
	};

	EllipseEmitter(uint32_t maxParticleNum = 131072) : kMaxParticleNum_(maxParticleNum) {};

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

	//1frameに1種類ずつ。Queue方式
	void Emit(const Config& config);

private:

	struct WellForGPU {
		Vector2 posA;
		Vector2 posB;
		float z;
		float radius;
		float lifeTime;
		float initialSpeed;
		Vector4 color;
		Matrix4x4 transform;
		uint32_t seed;
		uint32_t emitCount;
	};

	WellForGPU CreateConfigForGPU(const Config& config);

	std::vector<Config> emitQueue_ = {};

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;

	std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> emit_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;

	SHEngine::GPUBuffer* freeList_ = nullptr;
	SHEngine::GPUBuffer* freeListIndex_ = nullptr;
	SHEngine::GPUBuffer* indexList_ = nullptr;
	SHEngine::GPUBuffer* currentTime_ = nullptr;
	SHEngine::GPUBuffer* lifeTime_ = nullptr;
	SHEngine::GPUBuffer* velocities_ = nullptr;

	SHEngine::GPUBuffer* config_ = nullptr;
	SHEngine::GPUBuffer* maxParticleNum_ = nullptr;

	SHEngine::GPUBuffer* position_ = nullptr;
	SHEngine::GPUBuffer* color_ = nullptr;

	const uint32_t kMaxParticleNum_;
};
