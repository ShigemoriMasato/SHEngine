#include "SandScene.h"

SandScene::~SandScene() {
	Save();
}

void SandScene::Initialize() {
	camera_ = std::make_unique<DebugCamera>();
	camera_->Initialize(input_);

	Sand::InitializeData initData;
	initData.sandModel = modelManager_->GetModelData(SHEngine::TestModel::Cube);
	initData.particleCount = 10000000;
	initData.gridVerticalCount = 100;
	initData.gridHorizontalCount = 100;
	initData.gridDepthCount = 100;

	sandSimulator_ = std::make_unique<SandSimulator>(initData);
	sandSimulator_->Initialize(computeContext_);

	sandSimulator_->SetCamera(camera_.get());

	grid_.Initialize();

	Load();


	sandCPUSample_ = std::make_unique<SandCPUSample>(500, initData.sandModel);
	sandCPUSample_->Initialize();
	sandCPUSample_->SetCamera(camera_.get());
}

std::unique_ptr<IScene> SandScene::Update() {
	float deltaTime = engine_->GetDeltaTime();

	camera_->Update(commonData_->display->IsForcus());

	grid_.Update(camera_->GetCenter());

	if (isCPUSample_) {
		sandCPUSample_->Update(deltaTime);
	} else {
		sandSimulator_->Update(deltaTime, computeContext_);
	}

	auto fence = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(fence);

	return std::unique_ptr<IScene>();
}

void SandScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	grid_.Draw(directContext_);

	if (isCPUSample_) {
		sandCPUSample_->Draw(directContext_);
	} else {
		sandSimulator_->Draw(directContext_);
	}

	auto fence = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fence);

	display->ToTexture(directContext_);

	display->DrawImGui();
	sandSimulator_->DrawImGui();

#ifdef USE_IMGUI
	ImGui::Begin("System");
	float deltatime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %.2f", 1.0f / deltatime);
	ImGui::End();

	ImGui::Begin("Current Process");
	if (ImGui::Button((isCPUSample_ ? "CPU" : "GPU"))) {
		isCPUSample_ = !isCPUSample_;
		sandCPUSample_->Initialize();
	}
	ImGui::End();
#endif

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
