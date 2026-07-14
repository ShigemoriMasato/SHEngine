#include "PointRenderer.h"

void PointRenderer::Initialize(const Pool& pool, SHEngine::TextureManager* textureManager, SHEngine::Screen::IDisplay* renderTarget, const SHEngine::DrawData& pedd) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	for (int i = 0; i < 3; ++i) {
		auto& disp = intervalDisp_.emplace_back(std::make_unique<SHEngine::Screen::Display>());
		disp = std::make_unique<SHEngine::Screen::Display>();
		disp->Initialize(1280, 720, "Particle Interval");
		disp->AddRenderTarget(textureManager, 0x00000000, SHEngine::Format::R32_UINT, true);
	}

	redBuffer_ = container_->Create(intervalDisp_[0]->GetTextureData());
	greenBuffer_ = container_->Create(intervalDisp_[1]->GetTextureData());
	blueBuffer_ = container_->Create(intervalDisp_[2]->GetTextureData());
	vpMatrixBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));
	depthBuffer_ = container_->Create(renderTarget->GetDepthTexture());

	static constexpr int kThreadGroupSize = 128;
	static constexpr int kMaxGroupNum = 65535;
	const int totalThreadGroup = (pool.maxParticleNum + kThreadGroupSize - 1) / kThreadGroupSize;
	const int renderNum = totalThreadGroup / kMaxGroupNum + 1;
	for (int i = 0; i < renderNum; ++i) {
		//0: ExecuteOffset, 1: TotalParticleNum
		int offset[2] = { i * kMaxGroupNum * kThreadGroupSize, pool.maxParticleNum };
		auto executeOffsetBuffer = container_->Create(BufferType::CBV, sizeof(int) * 2);
		executeOffsetBuffer->CopyBuffer(offset, sizeof(int) * 2);

		auto& renderer = renderers_.emplace_back(std::make_unique<SHEngine::ComputeObject>());
		renderer->SetShader("Particle/Pool/Draw.CS.hlsl");
		renderer->SetGPUBuffers(BufferType::UAV, { redBuffer_, greenBuffer_, blueBuffer_ });
		renderer->SetGPUBuffers(BufferType::SRV, { depthBuffer_, pool.position, pool.color });
		renderer->SetGPUBuffers(BufferType::CBV, { executeOffsetBuffer, vpMatrixBuffer_ });

		int threadGroupX = kMaxGroupNum;
		if (i == renderNum - 1) {
			threadGroupX = totalThreadGroup % kMaxGroupNum;
		}
		renderer->SetThreadGroupSize(threadGroupX, 1, 1);
	}

	postEffect_ = std::make_unique<SHEngine::Renderer>(pedd);
	postEffect_->SetVS("PostEffect/PostEffect.VS.hlsl");
	postEffect_->SetPS("Game/Particle.PS.hlsl");
}

void PointRenderer::Update(const Matrix4x4& vpMatrix) {
	vpMatrixBuffer_->CopyBuffer(&vpMatrix, sizeof(Matrix4x4));
}

void PointRenderer::Draw(DCC* dcc, CCC* ccc) {

	for (auto& disp : intervalDisp_) {
		disp->ToUnordered(dcc, true);
	}

	//お互いの処理が終わるまで待つ
	auto fence = ccc->MiddleExecute();
	dcc->WaitFenceInGPU(fence);

	fence = dcc->MiddleExecute();
	ccc->WaitFenceInGPU(fence);

	for (auto& renderer : renderers_) {
		renderer->Execute(ccc);
	}
	fence = ccc->MiddleExecute();
	dcc->WaitFenceInGPU(fence);

	for (auto& disp : intervalDisp_) {
		disp->ToTexture(dcc);
	}

	//IntervalDispに描画された結果をRenderTargetにコピーする
}
