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

	CreateRenderer();

	pool_.maxParticleNum = kMaxParticleNum;

	drawCount_ = kMaxParticleNum / 64 / 2;
	drawCount_ = 60194;
}

void ParticlePool::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime, CCC* compute) {
	//drawCountに応じてレンダラーのDispatchGroupを設定する。
	int i = 0;
	for (i = 0; i < drawCount_ / 65535 + 1; ++i) {
		if (renderer_.size() <= i) {
			CreateRenderer();
		}
		auto& renderer = renderer_[i];
		renderer->SetDispatchGroup(std::min(((drawCount_)-i * 65535) + 1, 65535));
	}
	//余ったレンダラーのDispatchGroupを0にする
	for (i; i < renderer_.size(); ++i) {
		renderer_[i]->SetDispatchGroup(0);
	}

	camera_.vpMatrix = vpMatrix;
	camera_.billboardMatrix = billboardMatrix;
	vpMatrixBuffer_->CopyBuffer(&camera_, sizeof(camera_));
	sizeBuffer_->CopyBuffer(&size_, sizeof(size_));
	pool_.deltaTime->CopyBuffer(&deltaTime, sizeof(float));
}

void ParticlePool::Draw(DCC* dcc, CCC* ccc) {
	for (int i = 0; i < renderer_.size(); ++i) {
		renderer_[i]->Draw(dcc);
	}
}

void ParticlePool::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Particle Common");
	ImGui::DragFloat("Size", &size_, 0.01f);
	ImGui::SliderInt("RenderNum", &drawCount_, 1, pool_.maxParticleNum / 64);
	ImGui::End();

	sizeBuffer_->CopyBuffer(&size_, sizeof(size_));
#endif
}

void ParticlePool::CreateRenderer() {
	auto& renderer = renderer_.emplace_back(std::make_unique<SHEngine::MeshRenderer>());

	auto executeOffsetBuffer = container_->Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);
	int executeOffset = int(renderer_.size() - 1) * 65535 * 64;
	executeOffsetBuffer->CopyBuffer(&executeOffset, sizeof(executeOffset));

	renderer->SetMS("Mesh/Particle/Quad.MS.hlsl");
	renderer->SetPS("Game/GPUParticle.PS.hlsl");
	renderer->SetGPUBuffer(pool_.position, ShaderType::MESH_SHADER, BufferType::SRV);
	renderer->SetGPUBuffer(sizeBuffer_, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer->SetGPUBuffer(vpMatrixBuffer_, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer->SetGPUBuffer(pool_.particleNum, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer->SetGPUBuffer(executeOffsetBuffer, ShaderType::MESH_SHADER, BufferType::CBV);
	renderer->SetGPUBuffer(pool_.color, ShaderType::PIXEL_SHADER, BufferType::SRV);
	renderer->SetBlendState(SHEngine::PSO::BlendStateID::Add);
	renderer->SetDepthStencil(SHEngine::PSO::DepthStencilID::Default);
}
