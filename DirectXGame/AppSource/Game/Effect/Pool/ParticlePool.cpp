#include "ParticlePool.h"

void ParticlePool::Initialize(SHEngine::DrawData& planeDrawData, const int kMaxParticleNum, CmdObj* cmdObj) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	initialize_ = std::make_unique<SHEngine::ComputeObject>("Pool Init");
	update_ = std::make_unique<SHEngine::ComputeObject>("Pool Update");
	renderer_ = std::make_unique<SHEngine::Renderer>(planeDrawData);

	pool_.freeList = container_->Create(BufferType::SRV_UAV, sizeof(uint32_t), kMaxParticleNum); // freeList
	pool_.freeListIndex = container_->Create(BufferType::UAV, sizeof(uint32_t), 1); // freeListIndex
	pool_.position = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), kMaxParticleNum); // world
	pool_.color = container_->Create(BufferType::SRV_UAV, sizeof(Vector4), kMaxParticleNum); // color
	pool_.type = container_->Create(BufferType::SRV_UAV, sizeof(uint32_t), kMaxParticleNum); // type
	pool_.particleNum = container_->Create(BufferType::CBV, sizeof(uint32_t), 1); // particleNum
	pool_.particleNum->CopyBuffer(&kMaxParticleNum, sizeof(uint32_t));//定数なので初期化しておく
	pool_.deltaTime = container_->Create(BufferType::CBV, sizeof(float), 1); // deltaTime

	vpMatrixBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4) * 2, 1); // viewProjectionMatrix
	sizeBuffer_ = container_->Create(BufferType::CBV, sizeof(float), 1); // size
	auto wvpMatrix = container_->Create(BufferType::SRV_UAV, sizeof(Matrix4x4), kMaxParticleNum); // worldViewProjectionMatrix

	initialize_->SetShader("Particle/Pool/Initialize.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { pool_.freeList, pool_.freeListIndex, pool_.type });
	initialize_->SetGPUBuffer(BufferType::CBV, pool_.particleNum);
	initialize_->SetThreadGroupSize(kMaxParticleNum / kThreadGroupSize_);

	update_->SetShader("Particle/Pool/Update.CS.hlsl");
	update_->SetGPUBuffers(BufferType::UAV, { pool_.position, pool_.type, wvpMatrix });
	update_->SetGPUBuffers(BufferType::CBV, { vpMatrixBuffer_, pool_.particleNum, sizeBuffer_ });
	update_->SetThreadGroupSize(kMaxParticleNum / kThreadGroupSize_);

	renderer_->SetVS("Simples.VS.hlsl");
	renderer_->SetPS("Colors.PS.hlsl");
	renderer_->SetGPUBuffer(wvpMatrix, ShaderType::VERTEX_SHADER , BufferType::SRV);
	renderer_->SetGPUBuffer(pool_.color, ShaderType::PIXEL_SHADER, BufferType::SRV);
	renderer_->instanceNum_ = kMaxParticleNum;
	renderer_->SetBlendState(SHEngine::PSO::BlendStateID::Add);
	renderer_->SetUseTexture(true);

	initialize_->Execute(cmdObj);

	pool_.maxParticleNum = kMaxParticleNum;

	//めんどくさいので適当にdeltatimeぶち込んどく
	float deltaTime = 1.0f / 60.0f;
	pool_.deltaTime->CopyBuffer(&deltaTime, sizeof(deltaTime));
}

void ParticlePool::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, CmdObj* cmdObj) {
	camera_.vpMatrix = vpMatrix;
	camera_.billboardMatrix = billboardMatrix;
	vpMatrixBuffer_->CopyBuffer(&camera_, sizeof(camera_));
	sizeBuffer_->CopyBuffer(&size_, sizeof(size_));
	update_->Execute(cmdObj);
}

void ParticlePool::Draw(CmdObj* cmdObj) {
	renderer_->Draw(cmdObj);
}
