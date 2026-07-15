#pragma once
#include <Game/Effect/Base/IEmitter.h>
#include <random>

class PolygonEmitter : public IEmitter {
public:

	PolygonEmitter(uint32_t maxParticleNum = 1000000) : kMaxParticleNum_(maxParticleNum) {};

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

	uint32_t AddPolygon(const PolygonList& polygonList, Matrix4x4 worldMatrix = Matrix4x4::Identity(), Vector4 color = {1,1,1,1}, uint32_t emitNum = 1);

	void EditPolygon(uint32_t index, Matrix4x4 worldMatrix, Vector4 color, uint32_t emitNum);

private:

	std::unique_ptr<SHEngine::BufferContainer> container_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> update_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> emit_ = nullptr;
	std::unique_ptr<SHEngine::ComputeObject> addPolygon_ = nullptr;

	struct OneSet {
		SHEngine::GPUBuffer* polygonList;
		SHEngine::GPUBuffer* chanceListNum;
		SHEngine::GPUBuffer* chanceList;
		SHEngine::GPUBuffer* worldMatrix;
		SHEngine::GPUBuffer* color;
		SHEngine::GPUBuffer* emitNum;
		uint32_t emitNumValue;
	};

	std::map<uint32_t, OneSet> polygonSets_;
	uint32_t nextID_ = 0;

	SHEngine::GPUBuffer* maxParticleNum_ = nullptr;

	SHEngine::GPUBuffer* freeList_ = nullptr;
	SHEngine::GPUBuffer* freeListIndex_ = nullptr;
	SHEngine::GPUBuffer* indexList_ = nullptr;
	SHEngine::GPUBuffer* currentTime_ = nullptr;
	SHEngine::GPUBuffer* velocity_ = nullptr;

	SHEngine::GPUBuffer* lifeTime_ = nullptr;
	SHEngine::GPUBuffer* seed_ = nullptr;
	SHEngine::GPUBuffer* speed_ = nullptr;

	SHEngine::GPUBuffer* position_ = nullptr;
	SHEngine::GPUBuffer* color_ = nullptr;

	const uint32_t kMaxParticleNum_;
};
