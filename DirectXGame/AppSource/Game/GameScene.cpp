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
	worldCamera_ = gameCamera_->GetCamera();
	worldCamera_ = debugCamera_.get();
	worldCamera_ = manualCamera_.get();
}

void GameScene::Initialize() {
	keyCoating_ = std::make_unique<KeyCoating>(commonData_->keyManager.get());
	debugCamera_->Initialize(input_);

	finScene_ = std::make_unique<FinScene>(engine_);
	finScene_->PowerOff();

	effect_.Initialize(engine_);

	waveEmitter_ = std::make_unique<WaveEmitter>(6000000);
	effect_.AddEmitter(waveEmitter_.get());

	polygonEmitter_ = std::make_unique<PolygonEmitter>(6000000);
	effect_.AddEmitter(polygonEmitter_.get());

	rejectBallEmitter_ = std::make_unique<RejectBallPolygonEmitter>(6000000);
	effect_.AddEmitter(rejectBallEmitter_.get());

	fallPolygonEmitter_ = std::make_unique<FallPolygonEmitter>(1024);
	//effect_.AddEmitter(fallPolygonEmitter_.get());

	computeContext_->MiddleExecute();


	auto model = modelManager_->GetModelData(SHEngine::TestModel::Cube);	//Cube
	tetris_.Initialize(keyCoating_.get(), worldCamera_, model->meshes.front(), nullptr);

	//PostEffectの初期化
	postEffect_.Initialize(textureManager_);		//描画だけするやつなのでコピーオンリー
	postEffectConfig_.origin = commonData_->display.get();
	postEffectConfig_.jobs = uint32_t(PostEffectJob::None);

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);

	waveEmitterConfig_.textureID = textureManager_->LoadTexture("MagicCircle.png");

	rejectBallManager_.Initialize();

	model = modelManager_->GetModelData(SHEngine::TestModel::Plane);
	deleteLineMeshEffect_.Initialize(&tetris_, polygonEmitter_.get(), model->meshes.front());

	model = modelManager_->LoadModel("Pillar");
	polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), { 0.4f, 0.6f, 0.4f, 1.0f }, 800000);

	Load();

}

std::unique_ptr<IScene> GameScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	keyCoating_->Update(deltaTime);
	auto key = keyCoating_->GetKeyStates();
	Vector2 mousePos = commonData_->display->GetCursorPos(input_->GetCursorPos());

	waveEmitter_->SetConfig(waveEmitterConfig_);

	deleteLineMeshEffect_.SetParentMatrix(tetrisParent_.Matrix());
	deleteLineMeshEffect_.Update(deltaTime);

	rejectBallManager_.Update(deltaTime);
	rejectBallEmitter_->SetRejectBalls(rejectBallManager_.GetRejectBalls());

	computeContext_->BeginTimeStamp("Particle Update");
	effect_.Update(worldCamera_, deltaTime);
	computeContext_->EndTimeStamp();

	input_->Update();
	commonData_->keyManager->Update();

	debugCamera_->Update();
	gameCamera_->Update(deltaTime);

	if (!manualCamera_->UpdateCurve(deltaTime, false)) {
		deltaTime = 0.0f;
	}
	tetris_.SetParentMatrix(tetrisParent_.Matrix());
	tetris_.Update(deltaTime);

	//線を消したときのやつ
	int deleteNum = tetris_.IsLineDeleted();
	if (deleteNum || key[Key::Debug2]) {
		waveEmitter_->AddWave(waves_[deleteNum]);
		if (deleteNum > 2) {
			gameCamera_->Shake(0.3f * deleteNum, 0.5f);
		}
		rejectBallManager_.SetPresetFunc(RejectBallPreset::Impact);
	}

	if (key.at(Key::Debug1)) {
		return std::make_unique<GameScene>();
	}

	if ((tetris_.IsGameOver() && !prevIsGameOver_) || key.at(Key::Debug3)) {
		prevIsGameOver_ = true;
		finScene_->Initialize({0, 0, 0, 0.9f}, "Game Over", {1, 0, 0, 1});
	}

	FinSceneUI ans = FinSceneUI::None;
	ans = finScene_->Update(deltaTime, mousePos, key);

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

	timeViewer_->Add("Particle Update", computeContext_->GetTimeStampResult("Particle Update"));
	timeViewer_->Add("Particle Draw", directContext_->GetTimeStampResult("Particle Draw"));
	timeViewer_->Add("DeltaTime", engine_->GetDeltaTime());

	timeViewer_->Draw(directContext_);

	finScene_->Draw(directContext_);

	display->ToTexture(directContext_);

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
	//rejectBallManager_.DrawImGui();
	//tetris_.DrawImGui();
	//timeViewer_->DrawImGui();
	//finScene_->DrawImGui();
	deleteLineMeshEffect_.DrawImGui();

	ImGui::Begin("WaveEmitterConfig");
	waveEmitterConfig_.DrawImGui();
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

	finScene_->Save(bin);
	bin.Register(&tetrisParent_);

	deleteLineMeshEffect_.Save(bin);

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
	
	finScene_->Load(bin);
	tetrisParent_ = bin.Reverse<Transform>();
	deleteLineMeshEffect_.Load(bin);

	const std::string stageFile = "Game/StageConfig/" + commonData_->stageName_ + ".bin";
	if (!bin.Boot(stageFile)) {
		return;
	}

	CameraCurveData cameraCurveData;
	FallPolygonEmitter::MeshList fallMeshList;

	cameraCurveData.Load(bin);
	fallMeshList.Load(bin);

	manualCamera_->Inport(cameraCurveData);
	//fallPolygonEmitter_->AddPolygon(fallMeshList, modelManager_);

	//今回だけRejectBallのほうに情報を入れる
	for (const auto& meshInfo : fallMeshList.meshes) {
		auto model = modelManager_->LoadModel(meshInfo.modelPath);
		rejectBallEmitter_->AddPolygon(model->meshes, meshInfo.transform.Matrix(), meshInfo.color, meshInfo.emitNum);
	}
}
