#pragma once
#include <Game/Effect/Base/IEmitter.h>
#include <random>

struct PolygonData {
	Vector3 a;
	Vector3 b;
	Vector3 c;
};

struct PolygonList {
	std::vector<PolygonData> polygons;
	std::vector<float> areas;
	float totalArea = 0.0f;
};

class PolygonEmitter : public IEmitter {
public:

	struct Config {
		Config(uint32_t inID) : id(inID) {}
		const uint32_t id;
		Transform transform;
		Vector4 color = {1,1,1,1};
		uint32_t emitNum = 1;

		void DrawImGui();

		void Save(BinaryManager& bin) const;
		void Load(BinaryManager& bin);
	};

	struct IgnoreBall {
		Vector3 position = {0, 0, 0};
		float radius = 0.0f;
	};

public:

	PolygonEmitter(uint32_t maxParticleNum = 1000000) : kMaxParticleNum_(maxParticleNum) {};

	void Initialize(SHEngine::Engine* engine, const Pool& pool) override;
	void Update(CCC* compute, float deltaTime) override;

	Config AddPolygon(const std::vector<Mesh>& meshes, Matrix4x4 worldMatrix = Matrix4x4::Identity(), Vector4 color = {1,1,1,1}, uint32_t emitNum = 1);

	void SetConfig(const Config& config);
	void EditPolygon(uint32_t index, const PolygonList& polygonList, bool isCreateChanceList);

	void SetCommonConfig(float lifeTime);

	void SetIgnoreBalls(const std::array<IgnoreBall, 16>& ignoreBalls);

private:

	PolygonList CreatePolygonList(const std::vector<Mesh>& meshes);

	std::vector<int> CreateChanceList(const PolygonList& polygonList);

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
	SHEngine::GPUBuffer* basePosition_ = nullptr;

	SHEngine::GPUBuffer* lifeTime_ = nullptr;
	SHEngine::GPUBuffer* seed_ = nullptr;
	SHEngine::GPUBuffer* ignoreBallNum_ = nullptr;
	SHEngine::GPUBuffer* ignoreBall_ = nullptr;

	SHEngine::GPUBuffer* position_ = nullptr;
	SHEngine::GPUBuffer* color_ = nullptr;

	const uint32_t kMaxParticleNum_;
	const uint32_t kMaxIgnoreBallNum_ = 16;
};
