#include "TestScene.h"

TestScene::~TestScene() {
	Save();
}

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	gameCamera_ = std::make_unique<Camera>();
	gameCamera_->SetProjectionMatrix(PerspectiveFovDesc());

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	grid_ = std::make_unique<Grid>();
	grid_->Initialize();

	// ===================== 超えられない壁 =================================

	curveBinders_.resize(2);
	curveBinders_[0].BindVector3("Test1", &testParam_);
	curveBinders_[1].BindFloat("Test2", &testParam2_);

	curveEditor_.SetBinder(&curveBinders_[0]);
	curveEditor_.SetBinder(&curveBinders_[1]);

	container_ = std::make_unique<SHEngine::BufferContainer>();

	particleSystem_.Initialize(computeContext_);
	particleSystem_.SetCamera(debugCamera_.get());
	emitter_ = particleSystem_.CreateEmitter(1024 * 1024, computeContext_);

	auto model = modelManager_->GetModelData("Pyramid");
	Particle::InnerModel::EmitInnerModel(emitter_, container_.get(), 1024 * 1024, model, 0.1f, {0.3f, 0.3f, 0.3f, 0.3f});
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update(commonData_->display->IsForcus());

	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter());

	particleSystem_.Update(computeContext_);

	// ===================== 超えられない壁 =================================

	commonData_->display->DrawImGui();
	ImGuizmo::OriginalSetRect(commonData_->display.get());

	// ===================== 超えられない壁 =================================

	curveEditor_.Update(deltaTime);

	curveEditor_.DrawWindow();

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	{
		auto fence = computeContext_->MiddleExecute();
		directContext_->WaitFenceInCPU(fence);

		directContext_->SetRenderTarget(display);
		particleSystem_.Draw(directContext_);

		fence = directContext_->MiddleExecute();
		computeContext_->WaitFenceInCPU(fence);
	}

	directContext_->SetRenderTarget(display, false);

	grid_->SetCamera(debugCamera_.get());
	grid_->Draw(directContext_);

	// ↓↓↓ オブジェクト描画 ==============================================



	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

	directContext_->SetRenderTarget(window);
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = basePath_ + currentFileName_ + extension_;

	// ↓↓↓ 保存するデータ ==============================================

	// ↑↑↑ 保存するデータ ==============================================

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = basePath_ + currentFileName_ + extension_;

	if (!bin.Boot(fileName)) {
		return;
	}

}
