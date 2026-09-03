#include "TestScene.h"
#include <numbers>

#include <Utility/SearchFile.h>

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

	cameraEditor_.Initialize(gameCamera_.get());
	decoEditor_ = std::make_unique<DecoEditor>(engine_, commonData_->display.get());

	Load();

	{
		auto files = SearchFiles("Assets/Binary/" + basePath_, extension_);
		for (const auto& filePath : files) {
			std::filesystem::path path(filePath);
			std::string fileName = path.stem().string();
			fileList_.push_back(fileName);
		}
	}

	cameraEditor_.SetData(cameraCurveData_);
	decoEditor_->SetData(decoObjData_);

	models_.resize(16);
	for (auto& model : models_) {
		model = std::make_unique<ModelDrawer>();
	}
	auto model = modelManager_->GetModelData(SHEngine::TestModel::Field);
	models_[0]->Initialize(model, "Field");
	models_[0]->SetTransform({ Matrix4x4::Identity() });

	model = modelManager_->GetModelData(SHEngine::TestModel::Tower);
	models_[1]->Initialize(model, "Tower");
	models_[1]->SetTransform({ Matrix4x4::Identity() });

	stageEditor_.Initialize(textureManager_);
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


	for (auto& model : models_) {
		model->Update(debugCamera_.get(), deltaTime);
	}

	if (key[Key::Ctrl] && key[Key::S]) {
		Save();
	}

	if (key[Key::Ctrl] && key[Key::Z]) {
		decoEditor_->Undo();
	}

	if (key[Key::Ctrl] && key[Key::Y]) {
		decoEditor_->Redo();
	}

	stageEditor_.SetPresetFileList(fileList_);
	stageEditor_.Update();

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	directContext_->SetRenderTarget(display);
	grid_->SetCamera(debugCamera_.get());
	grid_->Draw(directContext_);

	directContext_->SetRenderTarget(display, false);

	decoEditor_->SetParentMatrix(Matrix::MakeRotationYMatrix(rotation_));

	// ↓↓↓ オブジェクト描画 ==============================================

	decoEditor_->Draw(directContext_);
	for (auto& model : models_) {
		model->Draw(directContext_);
	}

	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

	SelectFile();
	DrawInfo();

	directContext_->SetRenderTarget(window);
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	if (currentFileName_[0] == '\0') {
		return;
	}

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
	const float buttonSize = 80.f;
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

	auto Button = [&](const SHEngine::TextureData* iconTexture, const std::string& name)->bool {
		if (itemIndex % columnCount != 0) {
			ImGui::SameLine(0.0f, itemSpacing);
		}

		ImGui::BeginGroup();
		ImGui::ImageButton(name.c_str(), iconTexture->GetSRVHandle().ptr, ImVec2(buttonSize, buttonSize));
		const bool doubleClicked = ImGui::IsItemHovered() &&
			ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		const float textWidth = ImGui::CalcTextSize(name.c_str()).x;
		const float textOffset = std::max(0.0f, (tileWidth - textWidth) * 0.5f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);
		ImGui::TextUnformatted(name.c_str());
		ImGui::EndGroup();

		++itemIndex;

		return doubleClicked;
		};

	for (const auto& fileName : fileList_) {
		if (itemIndex % columnCount != 0) {
			ImGui::SameLine();
		}

		if (Button(fileTexture, fileName)) {
			std::memcpy(currentFileName_, fileName.c_str(), sizeof(currentFileName_));
			Load();
		}
	}

	ImGui::End();

	

#endif
}

void TestScene::DrawInfo() {
	const auto& data = decoEditor_->GetData();
	std::unordered_map<std::string, int> objectCount;

	for (const auto& [path, transforms] : data) {
		objectCount[path] = (int)transforms.size();
	}

#ifdef USE_IMGUI

	ImGui::Begin("Preset Info");

	ImGui::DragFloat("Rotation", &rotation_, 0.01f);
	rotation_ = std::fmod(rotation_, std::numbers::pi_v<float> * 2.0f);
	if (ImGui::Button("0")) {
		rotation_ = 0.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Half")) {
		rotation_ = std::numbers::pi_v<float> * 0.5f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rev")) {
		rotation_ = std::numbers::pi_v<float>;
	}
	ImGui::SameLine();
	if (ImGui::Button("1.5Rev")) {
		rotation_ = std::numbers::pi_v<float> * 1.5f;
	}

	for (const auto& [path, count] : objectCount) {
		ImGui::Text("%s: %d", path.c_str(), count);
	}

	ImGui::End();

#endif
}
