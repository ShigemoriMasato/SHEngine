#include "ParticlePool.h"
#include <imgui/imgui.h>

void ParticlePool::Initialize(const int kMaxParticleNum, CCC* compute, SHEngine::TextureManager* textureManager, SHEngine::Screen::IDisplay* renderTarget, const SHEngine::DrawData& pedd) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	initialize_ = std::make_unique<SHEngine::ComputeObject>("Pool Init");

	pool_.freeList = container_->Create(BufferType::UAV, sizeof(int), kMaxParticleNum, BufferNum::Single); // freeList
	pool_.freeListIndex = container_->Create(BufferType::UAV, sizeof(int), 1, BufferNum::Single); // freeListIndex

	pool_.position = container_->Create(BufferType::SRV_UAV, sizeof(Vector3), kMaxParticleNum, BufferNum::Single); // world
	pool_.color = container_->Create(BufferType::SRV_UAV, sizeof(Vector4) / 2, kMaxParticleNum, BufferNum::Single); // color(float16_t4)
	pool_.deltaTime = container_->Create(BufferType::CBV, sizeof(float), 1); // deltaTime
	pool_.particleNum = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single); // particleNum

	//定数なので初期化しておく
	pool_.particleNum->CopyBuffer(&kMaxParticleNum, sizeof(uint32_t));

	//描画用バッファ
	vpMatrixBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4) * 2, 1); // viewProjectionMatrix
	sizeBuffer_ = container_->Create(BufferType::CBV, sizeof(float), 1); // size

	initialize_->SetShader("Particle/Pool/Initialize.CS.hlsl");
	initialize_->SetGPUBuffers(BufferType::UAV, { pool_.freeList, pool_.freeListIndex, pool_.position });
	initialize_->SetGPUBuffer(BufferType::CBV, pool_.particleNum);
	initialize_->SetThreadGroupSize(kMaxParticleNum / kThreadGroupSize_ + 1);

	initialize_->Execute(compute);

	renderer_ = std::make_unique<SHEngine::MeshRenderer>();

	renderer_->SetAS("Mesh/Particle/Quad.AS.hlsl");
	renderer_->SetMS("Mesh/Particle/Quad.MS.hlsl");
	renderer_->SetPS("Game/GPUParticle.PS.hlsl");
	renderer_->SetGPUBuffer(pool_.position, ShaderType::MESH_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(sizeBuffer_, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(vpMatrixBuffer_, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(pool_.particleNum, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(pool_.color, ShaderType::PIXEL_SHADER, BufferType::SRV);
	renderer_->SetBlendState(SHEngine::PSO::BlendStateID::Add);
	renderer_->SetDepthStencil(SHEngine::PSO::DepthStencilID::Default);
	pool_.maxParticleNum = kMaxParticleNum;

	drawCount_ = kMaxParticleNum / 64 / 2;
	drawCount_ = 1;
}

void ParticlePool::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime, CCC* compute) {
	//drawCountに応じてレンダラーのDispatchGroupを設定する。
	static constexpr int kExecuteNum = 64 * 32;
	renderer_->SetDispatchGroup(drawCount_ / kExecuteNum + 1, 1, 1);

	camera_.vpMatrix = vpMatrix;
	camera_.billboardMatrix = billboardMatrix;
	vpMatrixBuffer_->CopyBuffer(&camera_, sizeof(camera_));
	sizeBuffer_->CopyBuffer(&size_, sizeof(size_));
	pool_.deltaTime->CopyBuffer(&deltaTime, sizeof(float));
}

void ParticlePool::Draw(DCC* dcc, CCC* ccc) {
	renderer_->Draw(dcc);
}

void ParticlePool::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Particle Common");
	ImGui::DragFloat("Size", &size_, 0.01f);
	ImGui::SliderInt("RenderNum", &drawCount_, 1, pool_.maxParticleNum);
	ImGui::End();

	sizeBuffer_->CopyBuffer(&size_, sizeof(size_));
#endif
}
