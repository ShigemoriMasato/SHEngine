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

	decoEditor_ = std::make_unique<DecoEditor>(engine_, commonData_->display.get());

	Load();
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	// ===================== 超えられない壁 =================================

	Vector2 mousePos = commonData_->display->GetCursorPos(input_->GetCursorPos());

	decoEditor_->Update(debugCamera_.get(), directContext_);


	if (input_->GetKeyState(DIK_LCONTROL)) {
		if(key[Key::Z]) {
			decoEditor_->Undo();
		}
		if (key[Key::Y]) {
			decoEditor_->Redo();
		}
	}

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();
	directContext_->SetRenderTarget(display);
	grid_->Draw(directContext_);


	// ↓↓↓ オブジェクト描画 ==============================================


	decoEditor_->Draw(directContext_);


	// ↑↑↑ オブジェクト描画 ==============================================


	display->ToTexture(directContext_);
	directContext_->SetRenderTarget(window);

#ifdef USE_IMGUI

	ImGui::Begin("FPS");
	float deltaTime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %f", 1.f / deltaTime);
	ImGui::End();

#endif
	
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	if (!bin.Boot(fileName)) {
		return;
	}

}
