#include "Effect.h"

void Effect::Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine) {
	engine_ = engine;
	textureManager_ = engine_->GetTextureManager();
	compute_ = engine_->GetComputeCommandContext();
	direct_ = engine_->GetDirectCommandContext();

	auto cmdObj = compute_->GetCurrentCmdObj();

	particlePool_ = std::make_unique<ParticlePool>();
	// メモリを確保する
	particlePool_->Initialize(planeDrawData, int(50000000), cmdObj);

	compute_->MiddleExecute();


}

void Effect::AddEmitter(IEmitter* emitter) {
	auto pool = particlePool_->GetPool();

	emitter->Initialize(engine_, pool, nextOffset_);
	nextOffset_ += emitter->GetMaxParticleNum();
	emitters_.push_back(emitter);
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	static Logger logger = GetLogger("Command");

	auto cmdObj = compute_->GetCurrentCmdObj();

	compute_->BeginTimeStamp("Particle Update");

	compute_->EndTimeStamp();

	//Queueに登録して実行
	auto fence = compute_->MiddleExecute();
	direct_->WaitFenceInGPU(fence);

	particlePool_->Update(vpMatrix, billboardMatrix, deltaTime);
}

void Effect::Draw(SHEngine::Screen::IDisplay* display) {
	//描画
	direct_->SetRenderTarget(display, true);
	particlePool_->Draw(direct_);

	//更新処理の後にこの関数を呼び出す
	direct_->MiddleExecute();

	particlePool_->DrawImGui();
}
