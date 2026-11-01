#include "BattleField.h"

BattleField::BattleField(ParticleSystem* particleSystem, SHEngine::Engine* engine, ImGuiWindowManager* imguiManager, ICC* icc) {
	auto mm = engine->GetModelManager();
	auto tm = engine->GetTextureManager();

	display_ = std::make_unique<SHEngine::Screen::Display>();
	display_->Initialize(textureSize_, textureSize_, "BattleField");
	display_->AddRenderTarget(tm, 0xff);	// 見た目
	display_->AddRenderTarget(tm, 0xff);		// 起伏

	emitter_ = particleSystem->CreateEmitter(textureSize_ * textureSize_, icc);
	container_ = std::make_unique<SHEngine::BufferContainer>();

	scale_ = container_->Create(BufferType::CBV, sizeof(float));
	heightScale_ = container_->Create(BufferType::CBV, sizeof(float));
	textureConfig_ = container_->Create(BufferType::CBV, sizeof(uint32_t) * 2);
	texture_ = container_->Create(display_->GetTextureAllData()[0]);
	heightTexture_ = container_->Create(display_->GetTextureAllData()[1]);
	uvs_ = container_->Create(BufferType::SRV_UAV, sizeof(Vector2), textureSize_ * textureSize_, BufferNum::Single);

	{
		auto white = tm->GetTextureData("Texture/white1x1.png");
		auto uv = tm->GetTextureData("Texture/uvChecker.png");
		testTexture_[0] = container_->Create(white);
		testTexture_[1] = container_->Create(uv);

		debugParamBuffer_ = container_->Create(BufferType::CBV, sizeof(int));
	}

	const float scale = 0.08f;
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
		ImGui::Image(ImTextureRef(display_->GetTextureData()->GetSRVHandle().ptr), { 64, 64 });
	}, "BattleField");
#endif

	update_ = emitter_->AddOprionalProcess("PixelTex/ApplyTex");
	update_->SetExecuteNum(textureSize_ * textureSize_, 128);
	update_->SetGPUBuffers(BufferType::SRV, { uvs_, texture_ });
	update_->SetGPUBuffers(BufferType::CBV, { heightScale_ });
	update_->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);

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
}

BattleField::~BattleField() {
}

void BattleField::Initialize() {
	SHEngine::ComputeObject* init = emitter_->AddTriggerProcess("PixelTex/Initialize");

	init->SetGPUBuffers(BufferType::CBV, { textureConfig_, scale_ });
	init->SetGPUBuffers(BufferType::SRV, { texture_ });
	init->SetGPUBuffers(BufferType::UAV, { uvs_ });
	init->SetExecuteNum(textureSize_ * textureSize_, 128);
	init->SetSamplerID(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);
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
	update_->Execute(icc);
}

void BattleField::Save() {
	BinaryManager manager;

	shootingStar_->Save(manager);

	manager.Write("BattleField");
}

void BattleField::Load() {
	BinaryManager manager;

	if (!manager.Boot("BattleField")) {
		return;
	}

	shootingStar_->Load(manager);
}
