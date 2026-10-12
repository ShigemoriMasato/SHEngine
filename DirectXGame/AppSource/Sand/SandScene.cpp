#include "SandScene.h"

SandScene::~SandScene() {
	Save();
}

void SandScene::Initialize() {
	camera_ = std::make_unique<DebugCamera>();
	camera_->Initialize(input_);

	Sand::InitializeData initData;
	initData.sandModel = modelManager_->GetModelData(SHEngine::TestModel::Cube);
	initData.particleCount = 10000;
	initData.gridVerticalCount = 100;
	initData.gridHorizontalCount = 100;
	initData.gridDepthCount = 100;

	sandSimulator_ = std::make_unique<SandSimulator>(initData);
	sandSimulator_->Initialize(computeContext_);

	sandSimulator_->SetCamera(camera_.get());

	grid_.Initialize();

	Load();
}

std::unique_ptr<IScene> SandScene::Update() {
	float deltaTime = engine_->GetDeltaTime();

	camera_->Update(commonData_->display->IsForcus());

	grid_.Update(camera_->GetCenter());

	sandSimulator_->Update(deltaTime, computeContext_);

	auto fence = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(fence);

	return std::unique_ptr<IScene>();
}

void SandScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	grid_.Draw(directContext_);
	sandSimulator_->Draw(directContext_);

	auto fence = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fence);

	display->ToTexture(directContext_);

	display->DrawImGui();
	sandSimulator_->DrawImGui();

	directContext_->SetRenderTarget(window);
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void SandScene::Save() const {
	BinaryManager bin;
	sandSimulator_->Save(&bin);
	bin.Write("SandSimulation.bin");
}

void SandScene::Load() {
	BinaryManager bin;
	if (bin.Boot("SandSimulation.bin")) {
		sandSimulator_->Load(&bin);
	}
}
