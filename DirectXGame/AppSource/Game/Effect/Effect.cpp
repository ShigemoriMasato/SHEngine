#include "Effect.h"

void Effect::Initialize(SHEngine::DrawData& planeDrawData, SHEngine::Engine* engine, SHEngine::TextureManager* textureManager) {
	engine_ = engine;
	textureManager_ = textureManager;

	compute_ = engine_->CreateCommandObject(SHEngine::Command::Type::Compute);
	direct_ = engine_->CreateCommandObject(SHEngine::Command::Type::Direct);

	compute_->ResetCommandList();

	particlePool_ = std::make_unique<ParticlePool>();
	// 16777216個分のメモリを確保する
	particlePool_->Initialize(planeDrawData, int(std::pow(2, 20)), compute_.get());

	//0にInitialize用のShaderが入っているため、実行して終わるまで待つ
	engine_->ExecuteCommand(SHEngine::Command::Type::Compute, { compute_.get() });

	//このプールを使用してパーティクルを発生させる
	auto pool = particlePool_->GetPool();

	//実行中に他のパーティクルの初期化(CPUだけ)を行う
	waveParticle_ = std::make_unique<WaveParticle>();
	uint32_t textureID = textureManager_->LoadTexture("WaveParticle.png");
	waveParticle_->Initialize(textureID, pool, 1);

	compute_->WaitForGPUIdle();

	subject_ = std::make_unique<Subject>();
	subject_->Initialize(engine);
}

void Effect::Update(const Matrix4x4& vpMatrix, const Matrix4x4& billboardMatrix, float deltaTime) {
	//全部のコマンドリストをリセット
	compute_->ResetCommandList();
	direct_->ResetCommandList();

	static Logger logger = getLogger("Command");
	logger->info(compute_->Log());

	//パーティクルの更新処理
	waveParticle_->Update(compute_.get(), deltaTime);

	//Queueに登録して実行
	computeFence_ = engine_->ExecuteCommand(SHEngine::Command::Type::Compute, { compute_.get() });

	particlePool_->Update(vpMatrix, billboardMatrix, deltaTime);

	subject_->Update(vpMatrix);
}

void Effect::Draw(SHEngine::Screen::IDisplay* display) {
	//描画
	direct_->SetRenderTarget(display, true);
	particlePool_->Draw(direct_.get());

	subject_->Draw(direct_.get());

	//更新処理の後にこの関数を呼び出す
	engine_->WaitFence(computeFence_, SHEngine::Command::Type::Direct);
	engine_->ExecuteCommand(SHEngine::Command::Type::Direct, { direct_.get() });

	particlePool_->DrawImGui();
	waveParticle_->DrawImGui();
}
