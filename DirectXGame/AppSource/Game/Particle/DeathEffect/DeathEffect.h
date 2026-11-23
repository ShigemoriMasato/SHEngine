#pragma once
#include <Particle/ParticleSystem.h>
#include <Assets/Model/ModelManager.h>

// 敵が死亡したときのエフェクトを生成するクラス。
class DeathEffect {
public:

	DeathEffect(ParticleSystem* particleSystem, SHEngine::ModelManager* modelManager, ICC* icc);
	void Initialize();

	void Update(float deltaTime);

	void DrawImGui();

private:

	struct PolygonData {
		SHEngine::GPUBuffer* vertex = nullptr;
		SHEngine::GPUBuffer* index = nullptr;
		SHEngine::GPUBuffer* chanceList = nullptr;
		SHEngine::GPUBuffer* chanceListSize = nullptr;
	};

	struct Buffers {
		SHEngine::GPUBuffer* emitCount = nullptr;
	};

	Particle::Emitter* emitter_ = nullptr;

	std::unique_ptr<SHEngine::BufferContainer> container_;

	SHEngine::GPUBuffer* initialPositions_ = nullptr;

	// 発生させるパーティクルの数
	const int emitCount_ = 1024;
	const int maxEmitCount_ = emitCount_ * 128;

	std::unordered_map<std::string, PolygonData> polygonDataMap_;
	std::unordered_map<std::string, int> emitCountMap_;

	std::unordered_map<std::string, Buffers> buffersMap_;
};
