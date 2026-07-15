#include "Effect.h"

void Effect::Initialize(SHEngine::Engine* engine, SHEngine::Screen::IDisplay* renderTarget, const SHEngine::DrawData& pedd) {
	engine_ = engine;
	textureManager_ = engine_->GetTextureManager();
	compute_ = engine_->GetComputeCommandContext();
	direct_ = engine_->GetDirectCommandContext();

	particlePool_ = std::make_unique<ParticlePool>();
	// メモリを確保する
	particlePool_->Initialize(int(100000000), compute_, textureManager_, renderTarget, pedd);

	compute_->MiddleExecute();
}

void Effect::AddEmitter(IEmitter* emitter) {
	auto pool = particlePool_->GetPool();

	emitter->Initialize(engine_, pool);
	emitters_.push_back(emitter);
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	static Logger logger = GetLogger("Command");

	for (auto& emitter : emitters_) {
		emitter->Update(compute_, deltaTime);
	}

	particlePool_->Update(vpMatrix, billboardMatrix, deltaTime, compute_);
}

void Effect::Draw() {
	auto fence = compute_->MiddleExecute();
	direct_->WaitFenceInGPU(fence);

	//描画
	particlePool_->Draw(direct_, compute_);

	fence = direct_->MiddleExecute();
	compute_->WaitFenceInGPU(fence);

	particlePool_->DrawImGui();
}
