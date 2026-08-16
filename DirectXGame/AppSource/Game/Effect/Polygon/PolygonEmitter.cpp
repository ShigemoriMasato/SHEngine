#include "PolygonEmitter.h"

void PolygonEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	CommonInitialize(engine, pool);

	update_ = std::make_unique<SHEngine::ComputeObject>();

	SetUpdate();

	emit_ = std::make_unique<SHEngine::ComputeObject>();
	emit_->SetShader("Particle/Polygon/Emit.CS.hlsl");
}


void PolygonEmitter::Update(CCC* compute, float deltaTime) {
	CommonUpdate(compute, deltaTime);

	update_->Execute(compute);
}

PolygonEmitter::Config PolygonEmitter::AddPolygon(const std::vector<Mesh>& meshes, Matrix4x4 worldMatrix, Vector4 color, uint32_t emitNum) {
	PolygonList polygonList = CreatePolygonList(meshes);
	std::vector<int> chanceList = CreateChanceList(polygonList);
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
	set.speed = container_->Create(BufferType::CBV, sizeof(float));

	//値のコピー
	set.chanceList->CopyBuffer(chanceList.data(), sizeof(int) * chanceListNum);
	set.chanceListNum->CopyBuffer(&chanceListNum, sizeof(int));
	set.polygonList->CopyBuffer(polygonList.polygons.data(), sizeof(PolygonData) * polygonList.polygons.size());
	set.worldMatrix->CopyBuffer(&worldMatrix, sizeof(Matrix4x4));
	set.color->CopyBuffer(&color, sizeof(Vector4));
	set.emitNumValue = emitNum;

	Config config(id);

	return config;
}

void PolygonEmitter::SetConfig(Config& config) {
	const auto& it = polygonSets_.find(config.id);
	if (it == polygonSets_.end()) {
		assert(false && "PolygonEmitter: SetConfig failed. index not found.");
		return;
	}

	auto& set = it->second;
	Matrix4x4 worldMat = config.transform.Matrix();
	set.worldMatrix->CopyBuffer(&worldMat, sizeof(Matrix4x4));
	set.color->CopyBuffer(&config.color, sizeof(Vector4));
	set.speed->CopyBuffer(&config.speed, sizeof(float));
	set.emitNumValue = config.emitNum;
}

void PolygonEmitter::EditPolygon(uint32_t index, const PolygonList& polygonList, bool isCreateChanceList) {
	const auto& it = polygonSets_.find(index);
	if (it == polygonSets_.end()) {
		assert(false && "PolygonEmitter: SetConfig failed. index not found.");
		return;
	}

	auto& set = it->second;
	set.polygonList->CopyBuffer(polygonList.polygons.data(), sizeof(PolygonData) * polygonList.polygons.size());

	if (isCreateChanceList) {
		auto chanceList = CreateChanceList(polygonList);
		int chanceListNum = static_cast<int>(chanceList.size());
		set.chanceList->CopyBuffer(chanceList.data(), sizeof(int) * chanceListNum);
		set.chanceListNum->CopyBuffer(&chanceListNum, sizeof(int));
	}
}

void PolygonEmitter::SetCommonConfig(float lifeTime) {
	lifeTime_->CopyBuffer(&lifeTime, sizeof(float));
}

void PolygonEmitter::CommonInitialize(SHEngine::Engine* engine, const Pool& pool) {
	if (kMaxParticleNum_ >= 65535 * 128) {
		throw std::runtime_error("PolygonEmitter: maxParticleNum is too large. It must be less than 65535 * 128.");
	}

	container_ = std::make_unique<SHEngine::BufferContainer>(64);

	maxParticleNum_ = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);		//定数はSingle
	maxParticleNum_->CopyBuffer(&kMaxParticleNum_, sizeof(uint32_t));

	freeList_ = container_->Create(BufferType::UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	freeListIndex_ = container_->Create(BufferType::UAV, sizeof(int), 1, BufferNum::Single);
	indexList_ = container_->Create(BufferType::SRV_UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	currentTime_ = container_->Create(BufferType::SRV_UAV, sizeof(float), kMaxParticleNum_, BufferNum::Single);
	basePosition_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), kMaxParticleNum_, BufferNum::Single);
	velocity_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), kMaxParticleNum_, BufferNum::Single);

	lifeTime_ = container_->Create(BufferType::CBV, sizeof(float));
	seed_ = container_->Create(BufferType::CBV, sizeof(uint32_t));

	deltaTime_ = pool.deltaTime;

	float initLifeTime = 2.0f;
	lifeTime_->CopyBuffer(&initLifeTime, sizeof(float));

	position_ = pool.position;
	color_ = pool.color;

	initialize_ = std::make_unique<SHEngine::ComputeObject>();
	initialize_->SetShader("Particle/EmitterInit.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, pool.freeList, pool.freeListIndex, indexList_ });
	initialize_->SetGPUBuffer(BufferType::CBV, maxParticleNum_);
	initialize_->SetExecuteNum(kMaxParticleNum_ / 1024 + 1);
	CCC* ccc = engine->GetComputeCommandContext();
	initialize_->Execute(ccc);
}

void PolygonEmitter::CommonUpdate(CCC* compute, float deltaTime) {
	uint32_t seed = GetRandU();
	seed_->CopyBuffer(&seed, sizeof(uint32_t));

	for (const auto& [id, set] : polygonSets_) {
		emit_->Initialize();
		emit_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, position_, color_, currentTime_, basePosition_, velocity_ });
		emit_->SetGPUBuffers(BufferType::SRV, { set.polygonList, set.chanceList, indexList_ });
		emit_->SetGPUBuffers(BufferType::CBV, { set.emitNum, set.color, set.worldMatrix, set.chanceListNum, seed_, set.speed });
		uint32_t emitNum = uint32_t(std::round(float(set.emitNumValue) * deltaTime));
		set.emitNum->CopyBuffer(&emitNum, sizeof(uint32_t));
		emit_->SetExecuteNum(std::min(65535, (int)emitNum / 128 + 1));
		emit_->Execute(compute);
	}
}

void PolygonEmitter::SetUpdate(std::vector<SHEngine::GPUBuffer*> uav, std::vector<SHEngine::GPUBuffer*> srv, std::vector<SHEngine::GPUBuffer*> cbv, std::string shader) {
	update_->Initialize();
	std::string shaderName = shader.empty() ? "Update.CS.hlsl" : shader;
	update_->SetShader("Particle/Polygon/" + shaderName);
	update_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, currentTime_, position_, color_ });
	update_->SetGPUBuffers(BufferType::SRV, { indexList_, basePosition_, velocity_ });
	update_->SetGPUBuffers(BufferType::CBV, { maxParticleNum_, lifeTime_, deltaTime_ });
	update_->SetExecuteNum(kMaxParticleNum_ / 128 + 1);

	update_->SetGPUBuffers(BufferType::UAV, uav);
	update_->SetGPUBuffers(BufferType::SRV, srv);
	update_->SetGPUBuffers(BufferType::CBV, cbv);
}

PolygonList PolygonEmitter::CreatePolygonList(const std::vector<Mesh>& meshes) {
	PolygonList polygonList;

	for (const auto& mesh : meshes) {
		if (mesh.indices.size() % 3 != 0) {
			throw std::runtime_error("PolygonEmitter: Mesh indices size is not a multiple of 3.");
		}

		for (uint32_t i = 0; i < mesh.indices.size(); i += 3) {
			PolygonData polygon;
			polygon.a = mesh.position[mesh.indices[i]];
			polygon.b = mesh.position[mesh.indices[i + 1]];
			polygon.c = mesh.position[mesh.indices[i + 2]];

			float area = 0.5f * MyMath::cross(polygon.b - polygon.a, polygon.c - polygon.a).Length();

			polygonList.polygons.push_back(polygon);
			polygonList.areas.push_back(area);
			polygonList.totalArea += area;
		}
	}

	return polygonList;
}

std::vector<int> PolygonEmitter::CreateChanceList(const PolygonList& polygonList) {
	std::vector<int> chanceList;
	chanceList.reserve(polygonList.polygons.size() * 5);

	//Particleの生成がせまい場所に偏るようであればこの値を大きくする。初期化が重いのであれば低くする
	const float chanceConstant = 10.0f;
	const float kChanceScale = (float)polygonList.polygons.size() * chanceConstant / polygonList.totalArea;
	for (int i = 0; i < polygonList.areas.size(); ++i) {
		const auto& area = polygonList.areas[i];
		uint32_t chance = std::max(1u, static_cast<uint32_t>(area * kChanceScale));
		chanceList.insert(chanceList.end(), chance, i);
	}

	return chanceList;
}

void PolygonEmitter::Config::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::PushID(id);
	ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform.position.x, 0.01f);
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::DragFloat("Speed", &speed, 0.01f);
	ImGui::DragInt("EmitNum", reinterpret_cast<int*>(&emitNum), 1, 0, 500000);
	ImGui::PopID();

#endif // USE_IMGUI
}

void PolygonEmitter::Config::Save(BinaryManager& bin) const {
	bin.Register(&transform);
	bin.Register(&color);
	bin.Register(&emitNum);
	bin.Register(&speed);
}

void PolygonEmitter::Config::Load(BinaryManager& bin) {
	transform = bin.Reverse<Transform>();
	color = bin.Reverse<Vector4>();
	emitNum = bin.Reverse<uint32_t>();
	speed = bin.Reverse<float>();
}
