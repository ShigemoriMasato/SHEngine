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

	effect_.Initialize(engine_);

	effect_.AddEmitter(&fallEmitter_);
	effect_.AddEmitter(&rejectBallEmitter_);

	model_ = modelManager_->LoadModel("Pyramid");
	fallConfig_ = fallEmitter_.AddPolygon(model_->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 0);
	rejectBallConfig_ = rejectBallEmitter_.AddPolygon(model_->meshes, Matrix4x4::Identity(), Vector4(1, 0, 0, 1), 0);

	auto sphere = modelManager_->GetModelData(SHEngine::TestModel::Sphere);
	modelDrawer_.Initialize(sphere);
	modelDrawer_.IsWireFrame(true);
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	// ===================== 超えられない壁 =================================

	if (emit_) {
		fallEmitter_.SetConfig(fallConfig_);
	}
	rejectBallEmitter_.SetConfig(rejectBallConfig_);
	rejectBallEmitter_.SetRejectBalls({ rejectBall_ });

	effect_.Update(debugCamera_.get(), deltaTime);

	modelDrawer_.Update(debugCamera_.get(), deltaTime);

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

	// ↑↑↑ オブジェクト描画 ==============================================

#ifdef USE_IMGUI

	ImGui::Begin("FallConfig");
	fallConfig_.DrawImGui();

	if (ImGui::Button("100k")) {
		fallConfig_.emitNum = 100000;
		emit_ = true;
	}

	ImGui::Separator();

	ImGui::DragFloat3("Gravity", &gravity_.x, 0.01f);
	ImGui::DragFloat("LifeTime", &lifeTime_, 0.01f, 0.0f);

	fallEmitter_.SetGravity(gravity_);
	fallEmitter_.SetLifeTime(lifeTime_);

	ImGui::End();

	ImGui::Begin("Sphere");
	ImGui::DragFloat3("Position", &fallSphere_.pos.x, 0.01f);
	ImGui::DragFloat("Radius", &fallSphere_.radius, 0.01f, 0.0f);
	if (ImGui::Button("Fall")) {
		fallEmitter_.Fall(fallSphere_);
	}
	ImGui::End();

	Vector3 scale = { fallSphere_.radius, fallSphere_.radius, fallSphere_.radius };
	modelDrawer_.SetTransform({ Matrix::MakeScaleMatrix(scale) * Matrix::MakeTranslationMatrix(fallSphere_.pos) });

	ImGui::Begin("RejectBallConfig");
	rejectBallConfig_.DrawImGui();
	ImGui::DragFloat3("Position", &rejectBall_.position.x, 0.01f);
	ImGui::DragFloat("Radius", &rejectBall_.radius, 0.01f, 0.0f);
	ImGui::End();

#endif

	display->ToTexture(directContext_);

#ifdef SH_RELEASE

	directContext_->SetRenderTarget(window, false);

#else

	directContext_->SetRenderTarget(window);

#endif

#ifdef USE_IMGUI

#endif
	
	display->DrawImGui();
	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	// ↓↓↓ 保存するデータ ==============================================



	// ↑↑↑ 保存するデータ ==============================================

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	if (!bin.Boot(fileName)) {
		return;
	}

}
