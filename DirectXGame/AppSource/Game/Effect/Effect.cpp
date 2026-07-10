#include "Effect.h"

void Effect::Initialize(SHEngine::Engine* engine) {
	engine_ = engine;
	textureManager_ = engine_->GetTextureManager();
	compute_ = engine_->GetComputeCommandContext();
	direct_ = engine_->GetDirectCommandContext();

	particlePool_ = std::make_unique<ParticlePool>();
	// メモリを確保する
	particlePool_->Initialize(int(50000000), compute_);

	compute_->MiddleExecute();
}

void Effect::AddEmitter(IEmitter* emitter) {
	auto pool = particlePool_->GetPool();

	emitter->Initialize(engine_, pool);
	emitters_.push_back(emitter);
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	static Logger logger = GetLogger("Command");

	particlePool_->Update(vpMatrix, billboardMatrix, deltaTime, compute_);

}

void Effect::Draw() {
	//描画
	particlePool_->Draw(direct_);

	particlePool_->DrawImGui();
}
