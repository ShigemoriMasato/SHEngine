#include "Effect.h"

void Effect::Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine) {
	engine_ = engine;
	textureManager_ = engine_->GetTextureManager();
	compute_ = engine_->GetComputeCommandContext();
	direct_ = engine_->GetDirectCommandContext();

	auto cmdObj = compute_->GetCurrentCmdObj();

	particlePool_ = std::make_unique<ParticlePool>();
	// 16777216個分のメモリを確保する
	particlePool_->Initialize(planeDrawData, int(30000000), cmdObj);

	compute_->MiddleExecute();

	//このプールを使用してパーティクルを発生させる
	auto pool = particlePool_->GetPool();

	waveParticle_ = std::make_unique<WaveParticle>();
	uint32_t textureID = textureManager_->LoadTexture("WaveParticle.png");
	waveParticle_->Initialize(textureManager_, pool, 1);
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	static Logger logger = GetLogger("Command");

	auto cmdObj = compute_->GetCurrentCmdObj();

	compute_->BeginTimeStamp("Particle Update");

	//パーティクルの更新処理
	waveParticle_->Update(cmdObj, deltaTime);

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
	//engine_->WaitFence(computeFence_, SHEngine::Command::Type::Direct);
	direct_->MiddleExecute();

	particlePool_->DrawImGui();
	waveParticle_->DrawImGui();
}
