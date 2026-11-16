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

	auto model = modelManager_->GetModelData(SHEngine::TestModel::Camera);
	cameraRenderer_.Initialize(model);

	curveBinders_.resize(2);
	curveBinders_[0].BindVector3("Test1", &testParam_);
	curveBinders_[1].BindFloat("Test2", &testParam2_);

	curveEditor_.SetBinder(&curveBinders_[0]);
	curveEditor_.SetBinder(&curveBinders_[1]);
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update(commonData_->display->IsForcus());

	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter());

	// ===================== 超えられない壁 =================================

	commonData_->display->DrawImGui();
	ImGuizmo::OriginalSetRect(commonData_->display.get());

	cameraRenderer_.SetTransform({ gameCamera_->GetViewMatrix().Inverse() });
	cameraRenderer_.Update(debugCamera_.get(), deltaTime);

	// ===================== 超えられない壁 =================================

	curveEditor_.Update(deltaTime);

	curveEditor_.DrawWindow();

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(display);
	grid_->SetCamera(debugCamera_.get());
	grid_->Draw(directContext_);

	directContext_->SetRenderTarget(display, false);

	// ↓↓↓ オブジェクト描画 ==============================================

	cameraRenderer_.Draw(directContext_);

	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

	SelectFile();

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

void TestScene::SelectFile() {
#ifdef USE_IMGUI

	ImGui::Begin("Select File");

	const SHEngine::TextureData* fileTexture = textureManager_->GetTextureData("Assets/.EngineResource/Texture/File.png");
	const auto windowSize = ImGui::GetContentRegionAvail();
	const float buttonSize = 64.f;
	const float itemSpacing = 8.0f;
	const float tileWidth = buttonSize + ImGui::GetStyle().FramePadding.x * 2.0f;
	const int columnCount = std::max(1, static_cast<int>((windowSize.x + itemSpacing) / (tileWidth + itemSpacing)));
	int itemIndex = 0;

	ImGui::InputText("File Name", currentFileName_, sizeof(currentFileName_));

	if (ImGui::Button("+")) {
		const auto& it = std::find(fileList_.begin(), fileList_.end(), currentFileName_);
		if (it == fileList_.end()) {
			fileList_.push_back(currentFileName_);
		}

		Save();
	}

	for (const auto& fileName : fileList_) {
		if (itemIndex % columnCount != 0) {
			ImGui::SameLine();
		}

		if (ImGui::ImageButton(fileName.c_str(), (ImTextureRef)fileTexture->GetSRVHandle().ptr, ImVec2(buttonSize, buttonSize))) {
			std::memcpy(currentFileName_, fileName.c_str(), sizeof(currentFileName_));
			Load();
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1), fileName.c_str());
	}

	ImGui::End();

#endif
}
