#include "TestScene.h"

TestScene::~TestScene() {
	Save();
}

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	grid_ = std::make_unique<Grid>();
	grid_->Initialize();

	// ===================== 超えられない壁 =================================

	selectBox_.StaticInitialize(engine_);
	selectBox_.Initialize("Retry");
	Load();
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	// ===================== 超えられない壁 =================================

	Vector2 mousePos = commonData_->display->GetCursorPos(input_->GetCursorPos());
	selectBox_.SetConfig(selectBoxConfig_);
	selectBox_.Update(deltaTime, &orthoCamera_, mousePos);

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();
	directContext_->SetRenderTarget(display);
	grid_->Draw(directContext_);


	// ↓↓↓ オブジェクト描画 ==============================================


	selectBox_.Draw(directContext_);


	// ↑↑↑ オブジェクト描画 ==============================================


	display->ToTexture(directContext_);
	directContext_->SetRenderTarget(window);

#ifdef USE_IMGUI

	selectBox_.DrawImGui();

	ImGui::Begin("Config");
	selectBoxConfig_.DrawImGui();
	ImGui::End();


	ImGui::Begin("FPS");
	float deltaTime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %f", 1.f / deltaTime);
	ImGui::End();

#endif

	commonData_->display->DrawImGui();
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	selectBoxConfig_.Save(bin);

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	if (!bin.Boot(fileName)) {
		return;
	}

	selectBoxConfig_.Load(bin);
}
