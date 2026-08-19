#include "FallPolygonEmitter.h"

void FallPolygonEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	PolygonEmitter::Initialize(engine, pool);
	isFall_ = container_->Create(BufferType::SRV_UAV, sizeof(uint32_t), kMaxParticleNum_, BufferNum::Single);
	gravity_ = container_->Create(BufferType::CBV, sizeof(Vector3));
	sphereNum_ = container_->Create(BufferType::CBV, sizeof(uint32_t));
	spheres_ = container_->Create(BufferType::SRV, sizeof(Sphere), kMaxSphereNum_);

	update_->Initialize();
	update_->SetShader("Particle/Polygon/Fall.CS.hlsl");
	update_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, currentTime_, position_, color_, velocity_, isFall_ });
	update_->SetGPUBuffers(BufferType::SRV, { indexList_, basePosition_ });
	update_->SetGPUBuffers(BufferType::CBV, { maxParticleNum_, lifeTime_, deltaTime_, gravity_ });
	update_->SetExecuteNum(kMaxParticleNum_ / 128 + 1);

	fallSphere_ = std::make_unique<SHEngine::ComputeObject>();
	fallSphere_->SetShader("Particle/Polygon/SphereColl.CS.hlsl");
	fallSphere_->SetGPUBuffers(BufferType::UAV, { isFall_, velocity_, color_ });
	fallSphere_->SetGPUBuffers(BufferType::SRV, { pool.position, spheres_, indexList_ });
	fallSphere_->SetGPUBuffers(BufferType::CBV, { maxParticleNum_, sphereNum_, seed_ });
}

void FallPolygonEmitter::Update(CCC* compute, float deltaTime) {
	if (!sphereList_.empty()) {
		uint32_t sphereNum = std::min((uint32_t)sphereList_.size(), kMaxSphereNum_);
		sphereNum_->CopyBuffer(&sphereNum, sizeof(uint32_t));
		spheres_->CopyBuffer(sphereList_.data(), sizeof(Sphere) * sphereNum);
		fallSphere_->SetExecuteNum(kMaxParticleNum_ / 128 + 1);
		fallSphere_->Execute(compute);
		sphereList_.clear();
	}

	PolygonEmitter::Update(compute, deltaTime);

}

void FallPolygonEmitter::AddPolygon(const MeshList& meshList, SHEngine::ModelManager* modelManager) {
	for (const auto& meshInfo : meshList.meshes) {
		auto modelData = modelManager->LoadModel(meshInfo.modelPath);
		PolygonEmitter::AddPolygon(modelData->meshes, meshInfo.transform.Matrix(), meshInfo.color, meshInfo.emitNum);
	}
}

void FallPolygonEmitter::SetConfig(Config& config) {
	config.emitNum *= 60;
	PolygonEmitter::SetConfig(config);
	config.emitNum = 0;
}

void FallPolygonEmitter::Fall(Sphere sphere) {
	sphereList_.push_back(sphere);
}

void FallPolygonEmitter::MeshList::Save(BinaryManager& bin) const {
	uint32_t meshNum = static_cast<uint32_t>(meshes.size());
	bin.Register(&meshNum);
	for (const auto& mesh : meshes) {
		bin.Register(&mesh.modelPath);
		bin.Register(&mesh.transform);
		bin.Register(&mesh.color);
		bin.Register(&mesh.emitNum);
	}
}

void FallPolygonEmitter::MeshList::Load(BinaryManager& bin) {
	uint32_t meshNum = bin.Reverse<uint32_t>();
	meshes.resize(meshNum);
	for (auto& mesh : meshes) {
		mesh.modelPath = bin.Reverse<std::string>();
		mesh.transform = bin.Reverse<Transform>();
		mesh.color = bin.Reverse<Vector4>();
		mesh.emitNum = bin.Reverse<uint32_t>();
	}
}
