#include "PolygonEmitter.h"

void PolygonEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	if (kMaxParticleNum_ >= 65535 * 128) {
		throw std::runtime_error("PolygonEmitter: maxParticleNum is too large. It must be less than 65535 * 128.");
	}

	container_ = std::make_unique<SHEngine::BufferContainer>();

	maxParticleNum_ = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);		//定数はSingle
	maxParticleNum_->CopyBuffer(&kMaxParticleNum_, sizeof(uint32_t));

	freeList_ = container_->Create(BufferType::UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	freeListIndex_ = container_->Create(BufferType::UAV, sizeof(int), 1, BufferNum::Single);
	indexList_ = container_->Create(BufferType::SRV_UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	currentTime_ = container_->Create(BufferType::SRV_UAV, sizeof(float), kMaxParticleNum_, BufferNum::Single);
	velocity_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector3) / 2, kMaxParticleNum_, BufferNum::Single);// float16_t3

	lifeTime_ = container_->Create(BufferType::CBV, sizeof(float));
	seed_ = container_->Create(BufferType::CBV, sizeof(uint32_t));
	speed_ = container_->Create(BufferType::CBV, sizeof(float));

	float initLifeTime = 1.0f;
	float initSpeed = 0.1f;
	lifeTime_->CopyBuffer(&initLifeTime, sizeof(float));
	speed_->CopyBuffer(&initSpeed, sizeof(float));

	position_ = pool.position;
	color_ = pool.color;

	initialize_ = std::make_unique<SHEngine::ComputeObject>();
	initialize_->SetShader("Particle/Polygon/Initialize.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, pool.freeList, pool.freeListIndex, indexList_ });
	initialize_->SetGPUBuffer(BufferType::CBV, maxParticleNum_);
	initialize_->SetThreadGroupSize(kMaxParticleNum_ / 1024);
	CCC* ccc = engine->GetComputeCommandContext();
	initialize_->Execute(ccc);

	update_ = std::make_unique<SHEngine::ComputeObject>();
	update_->SetShader("Particle/Polygon/Update.CS.hlsl");
	update_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, currentTime_, position_, color_ });
	update_->SetGPUBuffers(BufferType::SRV, { velocity_, indexList_ });
	update_->SetGPUBuffers(BufferType::CBV, { maxParticleNum_, lifeTime_, pool.deltaTime });
	update_->SetThreadGroupSize(kMaxParticleNum_ / 128 + 1);

	emit_ = std::make_unique<SHEngine::ComputeObject>();
	emit_->SetShader("Particle/Polygon/Emit.CS.hlsl");
}


void PolygonEmitter::Update(CCC* compute, float deltaTime) {
	uint32_t seed = randomDistribution_(randomEngine_);
	seed_->CopyBuffer(&seed, sizeof(uint32_t));

	for (const auto& [id, set] : polygonSets_) {
		emit_->Initialize();
		emit_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, position_, color_, velocity_, currentTime_ });
		emit_->SetGPUBuffers(BufferType::SRV, { set.polygonList, set.chanceList, indexList_ });
		emit_->SetGPUBuffers(BufferType::CBV, { set.emitNum, set.color, set.worldMatrix, speed_, set.chanceListNum, seed_ });
		emit_->SetThreadGroupSize(std::min(65535, (int)set.emitNumValue / 128 + 1));
		emit_->Execute(compute);
	}

	update_->Execute(compute);
}

uint32_t PolygonEmitter::AddPolygon(const PolygonList& polygonList, Matrix4x4 worldMatrix, Vector4 color, uint32_t emitNum) {
	std::vector<int> chanceList;

	//Particleの生成がせまい場所に偏る用であればこの値を上げること。
	const float kChanceScale = 1 / (polygonList.totalArea / (float)polygonList.polygons.size());
	for (int i = 0; i < polygonList.areas.size(); ++i) {
		const auto& area = polygonList.areas[i];
		uint32_t chance = std::max(1u, static_cast<uint32_t>(area * kChanceScale));
		chanceList.insert(chanceList.end(), chance, i);
	}
	int chanceListNum = static_cast<int>(chanceList.size());

	uint32_t id = nextID_++;
	OneSet& set = polygonSets_[id];

	//bufferの作成
	set.chanceList = container_->Create(BufferType::SRV, sizeof(int), chanceListNum, BufferNum::Single);
	set.chanceListNum = container_->Create(BufferType::CBV, sizeof(int), 1, BufferNum::Single);
	set.polygonList = container_->Create(BufferType::SRV, sizeof(PolygonData), (uint32_t)polygonList.polygons.size(), BufferNum::Single);
	set.worldMatrix = container_->Create(BufferType::CBV, sizeof(Matrix4x4));
	set.color = container_->Create(BufferType::CBV, sizeof(Vector4));
	set.emitNum = container_->Create(BufferType::CBV, sizeof(uint32_t));

	//値のコピー
	set.chanceList->CopyBuffer(chanceList.data(), sizeof(int) * chanceListNum);
	set.chanceListNum->CopyBuffer(&chanceListNum, sizeof(int));
	set.polygonList->CopyBuffer(polygonList.polygons.data(), sizeof(PolygonData) * polygonList.polygons.size());
	set.worldMatrix->CopyBuffer(&worldMatrix, sizeof(Matrix4x4));
	set.color->CopyBuffer(&color, sizeof(Vector4));
	set.emitNum->CopyBuffer(&emitNum, sizeof(uint32_t));
	set.emitNumValue = emitNum;

	return id;
}

void PolygonEmitter::EditPolygon(uint32_t index, Matrix4x4 worldMatrix, Vector4 color, uint32_t emitNum) {
	const auto& it = polygonSets_.find(index);
	if (it == polygonSets_.end()) {
		assert(false && "PolygonEmitter: EditPolygon failed. index not found.");
		return;
	}

	auto& set = it->second;
	set.worldMatrix->CopyBuffer(&worldMatrix, sizeof(Matrix4x4));
	set.color->CopyBuffer(&color, sizeof(Vector4));
	set.emitNum->CopyBuffer(&emitNum, sizeof(uint32_t));
	set.emitNumValue = emitNum;
}
