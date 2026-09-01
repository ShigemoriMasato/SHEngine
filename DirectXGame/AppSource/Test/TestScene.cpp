#include "TestScene.h"

TestScene::~TestScene() {
	Save();

	BinaryManager bin;
	uint32_t fileNum = static_cast<uint32_t>(fileList_.size());
	bin.Register(&fileNum);
	for (uint32_t i = 0; i < fileNum; ++i) {
		bin.Register(&fileList_[i]);
	}
	bin.Write("TestScene_Config.bin");
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

	cameraEditor_.Initialize(gameCamera_.get());
	decoEditor_ = std::make_unique<DecoEditor>(engine_, commonData_->display.get());

	Load();

	{
		BinaryManager bin;
		uint32_t fileNum = 0;
		if (bin.Boot("TestScene_Config.bin")) {
			fileNum = bin.Reverse<uint32_t>();
			fileList_.resize(fileNum);
			for (uint32_t i = 0; i < fileNum; ++i) {
				fileList_[i] = bin.Reverse<std::string>();
			}
		}
	}

	cameraEditor_.SetData(cameraCurveData_);
	decoEditor_->SetData(decoObjData_);

	auto model = modelManager_->GetModelData(SHEngine::TestModel::Camera);
	cameraRenderer_.Initialize(model);
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

	decoEditor_->Update(debugCamera_.get(), directContext_);

	cameraRenderer_.SetTransform({ gameCamera_->GetViewMatrix().Inverse() });
	cameraRenderer_.Update(debugCamera_.get(), deltaTime);

	if (key[Key::Ctrl] && key[Key::S]) {
		Save();
	}

	if (key[Key::Ctrl] && key[Key::Z]) {
		decoEditor_->Undo();
	}

	if (key[Key::Ctrl] && key[Key::Y]) {
		decoEditor_->Redo();
	}

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

	decoEditor_->Draw(directContext_);

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

	auto data = decoEditor_->GetData();
	Decorate::Save(data, bin);

	// ↑↑↑ 保存するデータ ==============================================

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = basePath_ + currentFileName_ + extension_;

	if (!bin.Boot(fileName)) {
		return;
	}

	DecoObjData data;
	Decorate::Load(data, bin);
	decoEditor_->SetData(data);
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
