#include "GameScene.h"
#include <imgui/imgui.h>
#include <Utility/Color.h>
#include <Title/TitleScene.h>

using namespace SHEngine;

namespace {
	
}

GameScene::GameScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	manualCamera_ = std::make_unique<Camera>();
	manualCamera_->SetProjectionMatrix(PerspectiveFovDesc());
	manualCamera_->MakeMatrix();
	gameCamera_ = std::make_unique<GameCamera>();

	gameCamera_->Initialize();
	worldCamera_ = debugCamera_.get();
	worldCamera_ = gameCamera_->GetCamera();
}

void GameScene::Initialize() {
	keyCoating_ = std::make_unique<KeyCoating>(commonData_->keyManager.get());
	debugCamera_->Initialize(input_);

	finScene_ = std::make_unique<FinScene>(engine_);
	finScene_->PowerOff();

	effect_.Initialize(engine_);

	waveEmitter_ = std::make_unique<WaveEmitter>(6000000);
	effect_.AddEmitter(waveEmitter_.get());

	polygonEmitter_ = std::make_unique<PolygonEmitter>();
	effect_.AddEmitter(polygonEmitter_.get());

	ellipseEmitter_ = std::make_unique<EllipseEmitter>();
	effect_.AddEmitter(ellipseEmitter_.get());

	computeContext_->MiddleExecute();


	auto model = modelManager_->GetModelData(SHEngine::TestModel::Cube);	//Cube
	auto ddsTexture = textureManager_->GetTextureData(textureManager_->LoadTexture("rostock_laage_airport_4k.dds"));
	tetris_.Initialize(keyCoating_.get(), worldCamera_, model->meshes.front(), ddsTexture);

	//PostEffectの初期化
	postEffect_.Initialize(textureManager_);		//描画だけするやつなのでコピーオンリー
	postEffectConfig_.origin = commonData_->display.get();
	postEffectConfig_.jobs = uint32_t(PostEffectJob::None);

	intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
	intermediateDisplay_->Initialize(1280, 720, "EdgeDetection");
	intermediateDisplay_->AddRenderTarget(textureManager_, 0xff);

	edgeDetection_ = std::make_unique<PostEffect>();
	edgeDetection_->Initialize(textureManager_);
	forEdgeDetection_.origin = commonData_->display.get();
	forEdgeDetection_.output = intermediateDisplay_.get();
	forEdgeDetection_.jobs = uint32_t(PostEffectJob::EdgeDetection);

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);

	waveEmitterConfig_.textureID = textureManager_->LoadTexture("MagicCircle.png");

	model = modelManager_->LoadModel("Pyramid");
	auto config = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
	polygonConfigs_.push_back(config);
	auto config2 = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
	polygonConfigs_.push_back(config2);

	ignoreBallManager_.Initialize();

	ignoreBalls_.resize(2);
	auto ballFunc1 = [this](float deltaTime, bool& destroyMe) -> PolygonEmitter::IgnoreBall {
		return ignoreBalls_[0];
		};
	auto ballFunc2 = [this](float deltaTime, bool& destroyMe) -> PolygonEmitter::IgnoreBall {
		return ignoreBalls_[1];
		};
	ignoreBallManager_.SetMove(ballFunc1);
	ignoreBallManager_.SetMove(ballFunc2);

	grayScale_.intensity = 1.0f;
	postEffect_.CopyBuffer(PostEffectJob::GrayScale, grayScale_);
	postEffectConfig_.jobs |= uint32_t(PostEffectJob::GrayScale);

	Outline outline;
	outline.edgeTextureIndex = intermediateDisplay_->GetTextureData()->GetHandle();
	outline.color = { 1.0f,0.1f,0.1f,1.0f };
	outline.strength = 0.7f;
	postEffect_.CopyBuffer(PostEffectJob::Outline, outline);
	postEffectConfig_.jobs |= uint32_t(PostEffectJob::Outline);

	Load();
}

std::unique_ptr<IScene> GameScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	keyCoating_->Update(deltaTime);
	auto key = keyCoating_->GetKeyStates();

	waveEmitter_->SetConfig(waveEmitterConfig_);
	for (const auto& config : polygonConfigs_) {
		polygonEmitter_->SetConfig(config);
	}

	ignoreBallManager_.Update(deltaTime);
	polygonEmitter_->SetIgnoreBalls(ignoreBallManager_.GetIgnoreBalls());

	computeContext_->BeginTimeStamp("Particle Update");
	effect_.Update(worldCamera_, deltaTime);
	computeContext_->EndTimeStamp();

	input_->Update();
	commonData_->keyManager->Update();

	debugCamera_->Update();
	gameCamera_->Update(deltaTime);

	tetris_.Update(deltaTime);

	//線を消したときのやつ
	int deleteNum = tetris_.IsLineDeleted();
	if (deleteNum || key[Key::Debug2]) {
		waveEmitter_->AddWave(waves_[deleteNum]);
		if (deleteNum > 2) {
			gameCamera_->Shake(0.3f * deleteNum, 0.5f);
		}
		ignoreBallManager_.SetPresetFunc(IgnoreBallPreset::Impact);
	}

	if (key.at(Key::Debug1)) {
		return std::make_unique<GameScene>();
	}

	if ((tetris_.IsGameOver() && !prevIsGameOver_) || key.at(Key::Debug3)) {
		prevIsGameOver_ = true;
		finScene_->Initialize({0, 0, 0, 0.9f}, "Game Over", {1, 0, 0, 1});
	}

	Vector2 mousePos = commonData_->display->GetCursorPos(input_->GetCursorPos());
	auto ans = finScene_->Update(deltaTime, mousePos, key);

	//ゲームオーバー時のUIでの選択肢によって次のシーンを発行する
	if (key[Key::Correct]) {
		switch (ans) {
		case FinSceneUI::Retry:
			return std::make_unique<GameScene>();
		case FinSceneUI::Title:
			return std::make_unique<TitleScene>();
		}
	}

	return nullptr;
}

void GameScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	tetris_.Draw(directContext_);
	if (tetris_.IsGameOver()) {
		//GameOverの文字を描画
	}

	//一番最後に描画
	directContext_->BeginTimeStamp("Particle Draw");
	effect_.Draw();
	directContext_->EndTimeStamp();

	//Effectで一度実行されるので、もう一度Setしなおす
	directContext_->SetRenderTarget(display, false);

	timeViewer_->Add("Particle Update", engine_->GetComputeCommandContext()->GetTimeStampResult("Particle Update"));
	timeViewer_->Add("Particle Draw", engine_->GetDirectCommandContext()->GetTimeStampResult("Particle Draw"));
	timeViewer_->Add("FPS", engine_->GetDeltaTime());

	timeViewer_->Draw(directContext_);

	finScene_->Draw(directContext_);

	display->ToTexture(directContext_);

	forEdgeDetection_.dcc = directContext_;
	int edgeDetectionTextureIndex = commonData_->display->GetDepthTexture()->GetHandle();
	edgeDetection_->CopyBuffer(PostEffectJob::EdgeDetection, edgeDetectionTextureIndex);
	edgeDetection_->Draw(forEdgeDetection_);
	intermediateDisplay_->DrawImGui();

	postEffectConfig_.dcc = directContext_;

#ifdef SH_RELEASE

	postEffectConfig_.output = window;
	postEffect_.Draw(postEffectConfig_);
	directContext_->SetRenderTarget(window, false);

#else

	postEffect_.Draw(postEffectConfig_);
	directContext_->SetRenderTarget(window);

#endif

	//ここ以外で記述する場合、ifdefを忘れないようにすること
#ifdef USE_IMGUI
	display->DrawImGui();
	//gameCamera_->DrawImGui();
	//ignoreBallManager_.DrawImGui();
	//tetris_.DrawImGui();
	//timeViewer_->DrawImGui();
	finScene_->DrawImGui();

	ImGui::Begin("WaveEmitterConfig");
	waveEmitterConfig_.DrawImGui();
	ImGui::End();

	ImGui::Begin("IgnoreBall");
	for (int i = 0; i < ignoreBalls_.size(); ++i) {
		ImGui::PushID(i);
		ImGui::DragFloat3("Position", &ignoreBalls_[i].position.x, 0.01f);
		ImGui::DragFloat("Radius", &ignoreBalls_[i].radius, 0.01f);
		ImGui::PopID();
		ImGui::Separator();
	}
	ImGui::End();

	ImGui::Begin("EllipseEmitter");
	if (ellipseConfig_.DrawImGui()) {
		ellipseEmitter_->Emit(ellipseConfig_);
	}
	ImGui::End();

	{
		ImGui::Begin("WaveData");
		static int waveIndex = 0;
		ImGui::Text("current: %d", waveIndex);
		if (ImGui::Button("-")) {
			waveIndex--;
		}
		ImGui::SameLine();
		if (ImGui::Button("+")) {
			waveIndex++;
		}
		waveIndex = std::clamp(waveIndex, 0, int(WaveType::Count) - 1);

		ImGui::Separator();

		if (waves_[waveIndex].DrawImGui()) {
			waveEmitter_->AddWave(waves_[waveIndex]);
		}
		ImGui::End();
	}

	{
		ImGui::Begin("PolygonEmitterConfig");
		static int polygonIndex = 0;
		ImGui::Text("current: %d", polygonIndex);
		if (ImGui::Button("-")) {
			polygonIndex--;
		}
		ImGui::SameLine();
		if (ImGui::Button("+")) {
			polygonIndex++;
		}
		polygonIndex = std::clamp(polygonIndex, 0, int(polygonConfigs_.size()) - 1);

		ImGui::Separator();

		polygonConfigs_[polygonIndex].DrawImGui();
		ImGui::End();
	}

	{
		ImGui::Begin("PolygonEmitterCommon");
		static float lifeTime = 1.0f;
		ImGui::DragFloat("LifeTime", &lifeTime, 0.01f);
		polygonEmitter_->SetCommonConfig(lifeTime);
		ImGui::End();
	}

#endif

	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void GameScene::Save() {
	BinaryManager bin;

	for (auto& wave : waves_) {
		wave.Save(bin);
	}
	waveEmitterConfig_.Save(bin);
	uint32_t polygonConfigSize = static_cast<uint32_t>(polygonConfigs_.size());
	bin.Register(&polygonConfigSize);
	for (auto& polygonConfig : polygonConfigs_) {
		polygonConfig.Save(bin);
	}
	uint32_t ignoreBallSize = static_cast<uint32_t>(ignoreBalls_.size());
	bin.Register(&ignoreBallSize);
	for (auto& ignoreBall : ignoreBalls_) {
		bin.Register(&ignoreBall.position);
		bin.Register(&ignoreBall.radius);
	}
	ellipseConfig_.Save(bin);

	finScene_->Save(bin);

	static const std::string fileName = "GameScene.bin";
	bin.Write(fileName);
}

void GameScene::Load() {
	BinaryManager bin;
	static const std::string fileName = "GameScene.bin";
	if (!bin.Boot(fileName)) {
		return;
	}

	for (auto& wave : waves_) {
		wave.Load(bin);
	}
	waveEmitterConfig_.Load(bin);
	uint32_t polygonConfigSize = bin.Reverse<uint32_t>();
	for (uint32_t i = 0; i < polygonConfigSize; ++i) {
		polygonConfigs_[i].Load(bin);
	}
	uint32_t ignoreBallSize = bin.Reverse<uint32_t>();
	ignoreBalls_.resize(ignoreBallSize);
	for (uint32_t i = 0; i < ignoreBallSize; ++i) {
		ignoreBalls_[i].position = bin.Reverse<Vector3>();
		ignoreBalls_[i].radius = bin.Reverse<float>();
	}
	ellipseConfig_.Load(bin);

	finScene_->Load(bin);
}
