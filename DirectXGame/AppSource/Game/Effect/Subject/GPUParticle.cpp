#include "GPUParticle.h"

void GPUParticle::Initialize(CCC* ccc, const SHEngine::DrawData& drawData) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	static constexpr uint32_t kMaxParticleNum = 1024;
	auto particleBuffer = container_->Create(BufferType::SRV_UAV, sizeof(Particle), kMaxParticleNum);
	vsDataBuffer_ = container_->Create(BufferType::CBV, sizeof(VSData));

	initialize_ = std::make_unique<SHEngine::ComputeObject>();
	initialize_->SetShader("Test/GPUParticleInitialize.CS.hlsl");
	initialize_->SetGPUBuffer(BufferType::UAV, particleBuffer);
	initialize_->SetThreadGroupSize(1, 1, 1);
	initialize_->Execute(ccc);

	update_ = std::make_unique<SHEngine::ComputeObject>();
	update_->SetShader("Test/GPUParticleUpdate.CS.hlsl");
	update_->SetGPUBuffer(BufferType::UAV, particleBuffer);
	update_->SetThreadGroupSize(1, 1, 1);

	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("Test/Particle/GPUParticle.VS.hlsl");
	renderer_->SetPS("Test/Particle/GPUParticle.PS.hlsl");
	renderer_->SetGPUBuffer(particleBuffer, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(vsDataBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetRasterizer(SHEngine::PSO::RasterizerID::CullNone);
	renderer_->instanceNum_ = kMaxParticleNum;
}

void GPUParticle::Update(CCC* ccc, const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	update_->Execute(ccc);

	VSData vsData{};
	vsData.vpMatrix = vpMatrix;
	vsData.billboardMatrix = billboardMatrix;
	vsDataBuffer_->CopyBuffer(&vsData, sizeof(VSData));

	float a = deltaTime;
	a += deltaTime;
}

void GPUParticle::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}
