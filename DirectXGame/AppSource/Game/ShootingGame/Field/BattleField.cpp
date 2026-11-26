#include "BattleField.h"

BattleField::BattleField(ParticleSystem* particleSystem, SHEngine::Engine* engine, ImGuiWindowManager* imguiManager, ICC* icc) {
	auto mm = engine->GetModelManager();
	auto tm = engine->GetTextureManager();

	display_ = std::make_unique<SHEngine::Screen::Display>();
	display_->Initialize(textureSize_, textureSize_, "BattleField");
	display_->AddRenderTarget(tm, 0xff);	// 見た目
	display_->AddRenderTarget(tm, 0xff);		// 起伏

	emitter_ = particleSystem->CreateEmitter(textureSize_ * textureSize_, icc, "BattleField");
	container_ = std::make_unique<SHEngine::BufferContainer>(32, "BattleField");

	scale_ = container_->Create(BufferType::CBV, sizeof(float));
	heightScale_ = container_->Create(BufferType::CBV, sizeof(float));
	textureConfig_ = container_->Create(BufferType::CBV, sizeof(uint32_t) * 2);
	texture_ = display_->GetTextureAllData()[0];
	heightTexture_ = display_->GetTextureAllData()[1];
	uvs_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector2), textureSize_ * textureSize_, BufferNum::Single);
	velocities_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector3) / 2, textureSize_ * textureSize_, BufferNum::Single);
	explosionConfig_ = container_->Create(BufferType::CBV, sizeof(ExplosionConfig));
	deltaTime_ = container_->Create(BufferType::CBV, sizeof(float));
	float  deltaTime = 1.f / 60.f;
	deltaTime_->CopyBuffer(&deltaTime, sizeof(float));

	{
		auto white = tm->GetTextureData("Texture/white1x1.png");
		auto uv = tm->GetTextureData("Texture/uvChecker.png");
		testTexture_[0] = white;
		testTexture_[1] = uv;

		debugParamBuffer_ = container_->Create(BufferType::CBV, sizeof(int));
	}

	const float scale = 0.1f;
	const float heightScale = 5.0f;
	const uint32_t textureConfig[2] = { textureSize_, textureSize_ };
	scale_->CopyBuffer(&scale, sizeof(float));
	heightScale_->CopyBuffer(&heightScale, sizeof(float));
	textureConfig_->CopyBuffer(textureConfig, sizeof(uint32_t) * 2);

#ifdef USE_IMGUI
	imguiManager->Add([this]() {
		ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f, 0.01f);
		ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Position", &transform_.position.x, 0.01f);

		ImGui::Separator();

		testConfig_.DrawImGui();

		if (ImGui::Button("Explosion")) {
			Explosion(testConfig_);
		}
		if (ImGui::Button("Initialize")) {
			Initialize();
		}

	}, "BattleField");
#endif

	// ComputeShaderの実行はComputeGraphで設定する。b0/t0/u0~u3(パーティクル共通)はエミッターが既定として割り当て済み
	initializeTrigger_ = emitter_->CreateTrigger("Initialize");
	initializeTrigger_->Bind("TextureConfig", textureConfig_);
	initializeTrigger_->Bind("Scale", scale_);
	initializeTrigger_->Bind("UVs", uvs_);

	explosionTrigger_ = emitter_->CreateTrigger("Explosion");
	explosionTrigger_->Bind("ExplosionConfig", explosionConfig_);
	explosionTrigger_->Bind("UVs", uvs_);
	explosionTrigger_->Bind("Velocities", velocities_);

	applyTexTrigger_ = emitter_->CreateTrigger("ApplyTex");
	applyTexTrigger_->Bind("HeightScale", heightScale_);
	applyTexTrigger_->Bind("DeltaTime", deltaTime_);
	applyTexTrigger_->Bind("UVs", uvs_);
	applyTexTrigger_->Bind("ColorTexture", texture_);
	applyTexTrigger_->Bind("HeightTexture", heightTexture_);
	applyTexTrigger_->Bind("Velocities", velocities_);

	BuildDefaultGraphs();

	engine_ = engine;

	OrthographicDesc desc;
	desc.right = static_cast<float>(textureSize_);
	desc.top = static_cast<float>(textureSize_);
	desc.bottom = 0.0f;
	camera_.SetProjectionMatrix(desc);
	camera_.MakeMatrix();

	{
		auto plane = mm->GetModelData(SHEngine::TestModel::Plane);
		shootingStar_ = std::make_unique<ShootingStar>(container_.get(), plane->meshes.front(), imguiManager);
		shootingStar_->Initialize(textureSize_, &camera_);
	}

	Load();

	testConfig_.seed = seedGenerator_(randomEngine_);
}

BattleField::~BattleField() {
	Save();
}

void BattleField::Initialize() {
	// 次のエミッターのUpdateで1回実行される
	emitter_->Request("Initialize");
}

void BattleField::BuildDefaultGraphs() {
	using SHEngine::SlotRequest;

	// 保存データがあればそちらを使う(ComputeGraphウィンドウで編集・保存したもの)
	if (SHEngine::ComputeGraph* graph = initializeTrigger_->GetGraph(); graph->HasNoDispatch()) {
		auto id = graph->AppendDispatch("Particle/ForGame/Field/Initialize.CS.hlsl", "Particles");
		graph->ConnectArgument(id, SlotRequest::Type::CBV, 1, "TextureConfig");
		graph->ConnectArgument(id, SlotRequest::Type::CBV, 2, "Scale");
		graph->ConnectArgument(id, SlotRequest::Type::UAV, 4, "UVs");
	}

	if (SHEngine::ComputeGraph* graph = explosionTrigger_->GetGraph(); graph->HasNoDispatch()) {
		auto id = graph->AppendDispatch("Particle/ForGame/Field/Explosion.CS.hlsl", "Particles");
		graph->ConnectArgument(id, SlotRequest::Type::CBV, 1, "ExplosionConfig");
		graph->ConnectArgument(id, SlotRequest::Type::SRV, 1, "UVs");
		graph->ConnectArgument(id, SlotRequest::Type::UAV, 4, "Velocities");
	}

	if (SHEngine::ComputeGraph* graph = applyTexTrigger_->GetGraph(); graph->HasNoDispatch()) {
		auto id = graph->AppendDispatch("Particle/ForGame/Field/ApplyTex.CS.hlsl", "Particles");
		graph->ConnectArgument(id, SlotRequest::Type::CBV, 1, "HeightScale");
		graph->ConnectArgument(id, SlotRequest::Type::CBV, 2, "DeltaTime");
		graph->ConnectArgument(id, SlotRequest::Type::SRV, 1, "UVs");
		graph->ConnectArgument(id, SlotRequest::Type::Texture, 2, "ColorTexture");
		graph->ConnectArgument(id, SlotRequest::Type::Texture, 3, "HeightTexture");
		graph->ConnectArgument(id, SlotRequest::Type::UAV, 4, "Velocities");
		graph->GetNode(id)->samplerID = uint32_t(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
	}
}

void BattleField::Update(float deltaTime) {
	emitter_->SetParentMatrix(transform_.Matrix());

	shootingStar_->Update(deltaTime);
}

void BattleField::Draw(DCC* dcc) {
	dcc->SetRenderTarget(display_.get());

	shootingStar_->Draw(dcc);

	display_->ToTexture(dcc);
}

void BattleField::ApplyTexture(ICC* icc) {
	debugParam_++;
	debugParamBuffer_->CopyBuffer(&debugParam_, sizeof(int));
	applyTexTrigger_->Fire(icc);
}

void BattleField::Explosion(const ExplosionConfig& config) {
	explosionConfig_->CopyBuffer(&config, sizeof(ExplosionConfig));

	// 次のエミッターのUpdateで1回実行される
	// (同じフレームに複数回呼ぶと、explosionConfig_は最後の値になる。GPUBufferの警告で検出できる)
	emitter_->Request("Explosion");
}

void BattleField::Save() {
	BinaryManager manager;

	shootingStar_->Save(manager);
	testConfig_.Save(manager);

	manager.Write("BattleField.bin");
}

void BattleField::Load() {
	BinaryManager manager;

	if (!manager.Boot("BattleField.bin")) {
		return;
	}

	shootingStar_->Load(manager);
	testConfig_.Load(manager);
}

void ExplosionConfig::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::PushID("ExplosionConfig");

	ImGui::DragFloat2("Position", &impactPos.x, 0.01f);
	ImGui::DragFloat("Radius", &radius, 0.01f);
	ImGui::DragFloat("Scatter", &scatter, 0.01f);
	ImGui::DragFloat("MinScatter", &minScatter, 0.01f);
	ImGui::DragFloat("HeightRange", &heightRange, 0.01f);
	ImGui::DragFloat("HeightOffset", &heightOffset, 0.01f);
	ImGui::DragFloat("RandForce", &randForce, 0.01f);
	ImGui::DragFloat("RandRange", &randRange, 0.01f);
	ImGui::DragFloat("RandHeightRange", &randHeightRange, 0.01f);

	ImGui::PopID();
#endif
}

void ExplosionConfig::Save(BinaryManager& manager) {
	manager.Register(impactPos);
	manager.Register(radius);
	manager.Register(scatter);
	manager.Register(minScatter);
	manager.Register(heightRange);
	manager.Register(heightOffset);
	manager.Register(randForce);
	manager.Register(randRange);
	manager.Register(randHeightRange);
}

void ExplosionConfig::Load(BinaryManager& manager) {
	manager.Reverse(impactPos);
	manager.Reverse(radius);
	manager.Reverse(scatter);
	manager.Reverse(minScatter);
	manager.Reverse(heightRange);
	manager.Reverse(heightOffset);
	manager.Reverse(randForce);
	manager.Reverse(randRange);
	manager.Reverse(randHeightRange);
}
