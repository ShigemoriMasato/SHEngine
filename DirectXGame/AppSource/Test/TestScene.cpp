#include "TestScene.h"

TestScene::~TestScene() {
	Save();
}

void TestScene::Initialize() {
	engine_->EditImGuiIni("TestScene");

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	grid_ = std::make_unique<Grid>();
	grid_->Initialize();

	// ===================== 超えられない壁 =================================

	{
		std::string testModelpath = "SneekWalk";
		modelDrawer_.Initialize(modelManager_->LoadModel(testModelpath));
		ModelDrawer::MaterialData material;
		material.textureIndex = textureManager_->GetWhite1x1Texture();
		modelDrawer_.SetMaterial({ material, material });
		skinningProcessor_.Initialize(modelManager_->LoadModel(testModelpath), &modelDrawer_);
		auto anim = modelManager_->LoadAnimation(testModelpath);
		modelDrawer_.SetAnimation(anim);
	}

	{
		effect_.Initialize(engine_);

		waveEmitter_ = std::make_unique<WaveEmitter>(6000000);
		effect_.AddEmitter(waveEmitter_.get());

		polygonEmitter_ = std::make_unique<PolygonEmitter>();
		effect_.AddEmitter(polygonEmitter_.get());

		waveEmitterConfig_.textureID = textureManager_->LoadTexture("MagicCircle.png");
	}

	{
		auto model = modelManager_->LoadModel("Pyramid");
		auto config = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
		polygonConfigs_.push_back(config);
		auto config2 = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
		polygonConfigs_.push_back(config2);
	}

	{
		std::vector<std::string> testModels = { "TestModel/MultiMaterial", "TestModel/MultiMesh" };

		for (uint32_t i = 0; i < int(testModels.size()); ++i) {
			const auto& modelPath = testModels[i];
			auto model = modelManager_->LoadModel(modelPath);
			auto& drawer = modelDrawers_.emplace_back(std::make_unique<ModelDrawer>());
			drawer->Initialize(model);
			Transform transform{};
			transform.position.y = 3.0f;
			transform.position.x = float(i) * 3.0f;
			drawer->SetTransform({ transform.Matrix() });
		}
	}

	Load();

	copy_.Initialize(textureManager_, true);
	copyConfig_.dcc = directContext_;
	copyConfig_.origin = commonData_->display.get();
	copyConfig_.output = commonData_->window.get();
	copyConfig_.jobs = 0;
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	// ===================== 超えられない壁 =================================

	static constexpr float kMoveSpeed = 3.0f;
	Vector2 velocity = { 0.0f, 0.0f };
	if (key[Key::Up]) {
		velocity.y += 1.f;
	}
	if (key[Key::Down]) {
		velocity.y -= 1.f;
	}
	if (key[Key::Right]) {
		velocity.x += 1.f;
	}
	if (key[Key::Left]) {
		velocity.x -= 1.f;
	}
	velocity = velocity.Normalize() * kMoveSpeed * deltaTime;
	modelTransform_.position.x += velocity.x;
	modelTransform_.position.z += velocity.y;

	waveEmitter_->SetConfig(waveEmitterConfig_);
	for (const auto& config : polygonConfigs_) {
		polygonEmitter_->SetConfig(config);
	}

	effect_.Update(debugCamera_.get(), deltaTime);

	modelDrawer_.SetTransform({ modelTransform_.Matrix() });
	modelDrawer_.Update(debugCamera_.get(), deltaTime);
	skinningProcessor_.Update(directContext_);

	for (auto& drawer : modelDrawers_) {
		drawer->Update(debugCamera_.get(), deltaTime);
	}

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();
	directContext_->SetRenderTarget(display);
	grid_->Draw(directContext_);

	effect_.Draw();

	directContext_->SetRenderTarget(display, false);

	// ↓↓↓ オブジェクト描画 ==============================================

	modelDrawer_.Draw(directContext_);

	for (auto& drawer : modelDrawers_) {
		drawer->Draw(directContext_);
	}

	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

#ifdef SH_RELEASE

	copy_.Draw(copyConfig_);
	directContext_->SetRenderTarget(window, false);

#else

	directContext_->SetRenderTarget(window);

#endif

#ifdef USE_IMGUI

	ImGui::Begin("Wave");
	waves_.DrawImGui();
	ImGui::End();

	ImGui::Begin("WaveEmitter");
	waveEmitterConfig_.DrawImGui();
	ImGui::End();

	ImGui::Begin("Polygon");
	static int currentPolygonIndex = 0;
	ImGui::Text("current: %d", currentPolygonIndex);
	if (ImGui::Button("-")) {
		currentPolygonIndex--;
	}
	ImGui::SameLine();
	if (ImGui::Button("+")) {
		currentPolygonIndex++;
	}
	currentPolygonIndex = std::clamp(currentPolygonIndex, 0, int(polygonConfigs_.size()) - 1);

	ImGui::Separator();

	auto& currentPolygonConfig = polygonConfigs_[currentPolygonIndex];
	currentPolygonConfig.DrawImGui();
	ImGui::End();
	

	ImGui::Begin("FPS");
	float deltaTime = engine_->GetDeltaTime();
	ImGui::Text("FPS: %f", 1.f / deltaTime);
	ImGui::End();

#endif
	
	display->DrawImGui();
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	// ↓↓↓ 保存するデータ ==============================================

	waves_.Save(bin);
	int polygonSize = static_cast<int>(polygonConfigs_.size());
	bin.Register(&polygonSize);
	for (int i = 0; i < polygonSize; ++i) {
		polygonConfigs_[i].Save(bin);
	}
	waveEmitterConfig_.Save(bin);

	// ↑↑↑ 保存するデータ ==============================================

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	if (!bin.Boot(fileName)) {
		return;
	}

	waves_.Load(bin);
	int polygonSize = bin.Reverse<int>();
	for (int i = 0; i < polygonSize; ++i) {
		polygonConfigs_[i].Load(bin);
	}
	waveEmitterConfig_.Load(bin);
}
