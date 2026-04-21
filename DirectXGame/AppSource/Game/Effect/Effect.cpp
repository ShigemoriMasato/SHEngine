#include "Effect.h"

void Effect::Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine) {
	engine_ = engine;

	computeFence_.resize(6);
	compute_.resize(6);
	for (int i = 0; i < 6; ++i) {
		compute_[i] = engine_->CreateCommandObject(SHEngine::Command::Type::Compute, i);
	}
	update_ = engine_->CreateCommandObject(SHEngine::Command::Type::Compute, 0);
	direct_ = engine_->CreateCommandObject(SHEngine::Command::Type::Direct);

	compute_[0]->ResetCommandList();

	particlePool_ = std::make_unique<ParticlePool>();
	// 1048576個分のメモリを確保する
	particlePool_->Initialize(planeDrawData, int(std::pow(2, 20)), compute_[0].get());

	//0にInitialize用のShaderが入っているため、実行して終わるまで待つ
	engine_->ExecuteCommand(SHEngine::Command::Type::Compute, 0, { compute_[0].get() });

	//このプールを使用してパーティクルを発生させる
	auto pool = particlePool_->GetPool();

	//実行中に他のパーティクルの初期化(CPUだけ)を行う
	waveParticle_ = std::make_unique<WaveParticle>();
	waveParticle_->Initialize(pool, 1);

	compute_[0]->WaitForGPUIdle();
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix) {
	//全部のコマンドリストをリセット
	for (const auto& compute : compute_) {
		compute->ResetCommandList();
	}
	update_->ResetCommandList();
	direct_->ResetCommandList();

	//前フレームの描画処理が終わるまで待つ
	for (uint32_t i = 0; i < 6; ++i) {
		engine_->WaitFence(drawFence_, SHEngine::Command::Type::Direct, i);
	}

	//あらゆるパーティクルの更新を行う
	waveParticle_->Update(compute_[0].get());

	//Commandを実行
	for (size_t i = 0; i < compute_.size(); ++i) {
		computeFence_[i] = engine_->ExecuteCommand(SHEngine::Command::Type::Compute, uint32_t(i), {compute_[i].get()});
	}
	//次の更新処理を0で行うので、0のキューが全てのGPU処理が終わるまで待つ
	for (size_t i = 0; i < computeFence_.size(); ++i) {
		engine_->WaitFence(computeFence_[i], SHEngine::Command::Type::Compute, 0);
	}

	//最後に実行
	particlePool_->Update(vpMatrix, billboardMatrix, update_.get());
	engine_->ExecuteCommand(SHEngine::Command::Type::Compute, 0, { update_.get() });
}

void Effect::Draw(SHEngine::Screen::IDisplay* display) {
	//描画
	direct_->SetRenderTarget(display, true);
	particlePool_->Draw(direct_.get());

	//この描画処理が終わる前にCSが走らないようにFenceを取得しておく
	drawFence_ = engine_->ExecuteCommand(SHEngine::Command::Type::Direct, 0, { direct_.get() });
}
