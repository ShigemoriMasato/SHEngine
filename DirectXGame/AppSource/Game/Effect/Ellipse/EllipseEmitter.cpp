#include "EllipseEmitter.h"

void EllipseEmitter::Initialize(SHEngine::Engine* engine, const Pool& pool) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	maxParticleNum_ = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);		//定数はSingle
	maxParticleNum_->CopyBuffer(&kMaxParticleNum_, sizeof(uint32_t));

	freeList_ = container_->Create(BufferType::UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	freeListIndex_ = container_->Create(BufferType::UAV, sizeof(int), 1, BufferNum::Single);
	indexList_ = container_->Create(BufferType::SRV_UAV, sizeof(int), kMaxParticleNum_, BufferNum::Single);
	currentTime_ = container_->Create(BufferType::UAV, sizeof(float), kMaxParticleNum_, BufferNum::Single);
	lifeTime_ = container_->Create(BufferType::SRV_UAV, sizeof(float), kMaxParticleNum_, BufferNum::Single);
	velocities_ = container_->Create(BufferType::UAV, sizeof(Vector3), kMaxParticleNum_, BufferNum::Single);

	config_ = container_->Create(BufferType::CBV, sizeof(WellForGPU));
	
	position_ = pool.position;
	color_ = pool.color;

	initialize_ = std::make_unique<SHEngine::ComputeObject>();
	initialize_->SetShader("Particle/EmitterInit.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, pool.freeList, pool.freeListIndex, indexList_ });
	initialize_->SetGPUBuffer(BufferType::CBV, maxParticleNum_);
	initialize_->SetExecuteNum(kMaxParticleNum_ / 1024 + 1);
	auto ccc = engine->GetComputeCommandContext();
	initialize_->Execute(ccc);

	emit_ = std::make_unique<SHEngine::ComputeObject>();
	emit_->SetShader("Particle/Ellipse/Emit.CS.hlsl");

	update_ = std::make_unique<SHEngine::ComputeObject>();
	update_->SetShader("Particle/Ellipse/Update.CS.hlsl");
	update_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, currentTime_, velocities_, position_, color_ });
	update_->SetGPUBuffers(BufferType::SRV, { indexList_, lifeTime_ });
	update_->SetGPUBuffers(BufferType::CBV, { maxParticleNum_, pool.deltaTime });
	update_->SetExecuteNum(kMaxParticleNum_ / 128 + 1);
}

void EllipseEmitter::Update(CCC* compute, float deltaTime) {
	if (!emitQueue_.empty()) {
		auto& config = emitQueue_.front();
		config.emitCount = std::min(config.emitCount, kMaxParticleNum_);
		WellForGPU configForGPU  = CreateConfigForGPU(config);

		config_->CopyBuffer(&configForGPU, sizeof(WellForGPU));

		emit_->Initialize();
		emit_->SetExecuteNum(config.emitCount / 128 + 1);

		emit_->SetGPUBuffers(BufferType::UAV, { freeList_, freeListIndex_, currentTime_, lifeTime_, position_, color_, velocities_ });
		emit_->SetGPUBuffers(BufferType::SRV, { indexList_ });
		emit_->SetGPUBuffers(BufferType::CBV, { config_ });

		emit_->Execute(compute);

		emitQueue_.erase(emitQueue_.begin());
	}

	update_->Execute(compute);
}

void EllipseEmitter::Emit(const Config& config) {
	emitQueue_.push_back(config);
}

EllipseEmitter::WellForGPU EllipseEmitter::CreateConfigForGPU(const Config& config) {
	WellForGPU output;
	output.posA = config.posA;
	output.posB = config.posB;
	output.z = config.z;
	output.radius = config.radius;
	output.initialSpeed = config.initialSpeed;
	output.lifeTime = config.lifeTime;
	output.color = config.color;
	output.transform = config.transform.Matrix();
	output.emitCount = config.emitCount;
	output.seed = GetRandU();
	return output;
}

bool EllipseEmitter::Config::DrawImGui() {
	bool trigger = false;
#ifdef USE_IMGUI
	ImGui::DragFloat2("PosA", &posA.x, 0.1f);
	ImGui::DragFloat2("PosB", &posB.x, 0.1f);
	ImGui::DragFloat("Z", &z, 0.1f);
	ImGui::DragFloat("Radius", &radius, 0.1f);
	ImGui::DragFloat("LifeTime", &lifeTime, 0.01f);
	ImGui::DragFloat("InitialSpeed", &initialSpeed, 0.01f);
	ImGui::ColorEdit4("Color", &color.x);
	ImGui::DragFloat3("TransformScale", &transform.scale.x, 0.01f);
	ImGui::DragFloat3("TransformRotate", &transform.rotate.x, 0.01f);
	ImGui::DragFloat3("TransformPosition", &transform.position.x, 0.01f);
	ImGui::DragInt("EmitCount", (int*)&emitCount, 1, 1, 100000);
	if (ImGui::Button("Emit")) {
		trigger = true;
	}
#endif
	return trigger;
}

void EllipseEmitter::Config::Save(BinaryManager& binaryManager) const {
	binaryManager.Register<Vector2>(&posA);
	binaryManager.Register<Vector2>(&posB);
	binaryManager.Register<float>(&z);
	binaryManager.Register<float>(&radius);
	binaryManager.Register<float>(&lifeTime);
	binaryManager.Register<float>(&initialSpeed);
	binaryManager.Register<Vector4>(&color);
	binaryManager.Register<Transform>(&transform);
	binaryManager.Register<uint32_t>(&emitCount);
}

void EllipseEmitter::Config::Load(BinaryManager& binaryManager) {
	posA = binaryManager.Reverse<Vector2>();
	posB = binaryManager.Reverse<Vector2>();
	z = binaryManager.Reverse<float>();
	radius = binaryManager.Reverse<float>();
	lifeTime = binaryManager.Reverse<float>();
	initialSpeed = binaryManager.Reverse<float>();
	color = binaryManager.Reverse<Vector4>();
	transform = binaryManager.Reverse<Transform>();
	emitCount = binaryManager.Reverse<uint32_t>();
}
