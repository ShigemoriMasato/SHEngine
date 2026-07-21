#include "GameScene.h"
#include <imgui/imgui.h>
#include <Utility/Color.h>
#include <Title/TitleScene.h>

using namespace SHEngine;

namespace {
	void PostEffectImGui(PostEffect* postEffect, PostEffectConfig& config, TextureManager* tm, Screen::IDisplay* edgeTexture) {
		ImGui::Begin("PostEffect");
		static bool grayScale = false;
		static bool vignette = false;
		static bool boxBlur = false;
		static bool gaussBlur = false;
		static bool edgeDetection = false;
		static bool outline = false;
		static bool radialBlur = false;
		static bool dissolve = false;
		ImGui::Checkbox("GrayScale", &grayScale);
		if (grayScale) {
			static Grayscale config;
			ImGui::PushID("GrayScale");
			ImGui::DragFloat("intensity", &config.intensity, 0.01f);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::GrayScale, config);
		}
		ImGui::Checkbox("Vignette", &vignette);
		if (vignette) {
			static Vignette config;
			ImGui::PushID("Vignette");
			ImGui::ColorEdit4("Color", &config.color.x);
			ImGui::DragFloat("Strength", &config.intensity, 0.01f);
			ImGui::DragFloat("lerpWidth", &config.radius, 0.01f);
			ImGui::DragFloat("softness", &config.softness, 0.01f);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::Vignette, config);
		}
		ImGui::Checkbox("BoxBlur", &boxBlur);
		if (boxBlur) {
			static Blur config;
			ImGui::PushID("BoxBlur");
			ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 30);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::BoxBlur, config);
		}
		ImGui::Checkbox("GaussBlur", &gaussBlur);
		if (gaussBlur) {
			static GaussBlur config;
			ImGui::PushID("GaussBlur");
			ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 15);
			ImGui::DragFloat("Sigma", &config.sigma, 0.01f);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::GaussBlur, config);
		}
		ImGui::Checkbox("EdgeDetection", &edgeDetection);
		if (edgeDetection) {
			//データは必要ないため無記入
		}
		ImGui::Checkbox("Outline", &outline);
		if (outline) {
			static Outline config;
			config.edgeTextureIndex = edgeTexture->GetTextureData()->GetHandle();
			ImGui::PushID("Outline");
			ImGui::ColorEdit4("Color", &config.color.x);
			ImGui::DragFloat("Strength", &config.strength, 0.01f);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::Outline, config);
		}
		ImGui::Checkbox("RadialBlur", &radialBlur);
		if (radialBlur) {
			static RadialBlur config;
			ImGui::PushID("RadialBlur");
			ImGui::DragFloat2("Center", &config.center.x, 0.01f);
			ImGui::DragFloat("Strength", &config.strength, 0.01f);
			ImGui::DragInt("SampleCount", &config.sampleCount, 1, 1, 10);
			ImGui::PopID();
			postEffect->CopyBuffer(PostEffectJob::RadialBlur, config);
		}
		ImGui::Checkbox("Dissolve", &dissolve);
		if (dissolve) {
			static std::vector<int> noise = {
				tm->LoadTexture("Noise0.png"),
				tm->LoadTexture("Noise1.png")
			};
			static Dissolve config;
			static int noiseIndex = 0;
			ImGui::PushID("Dissolve");
			ImGui::SliderInt("NoiseTexture", &noiseIndex, 0, int(noise.size()) - 1);
			ImGui::SliderFloat("Threshold", &config.threshold, 0.0f, 1.0f);
			ImGui::DragFloat("EdgeThreshold", &config.edgeThreshold, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("EdgeColor", &config.edgeColor.x);
			ImGui::PopID();
			config.noiseTextureIndex = noise[noiseIndex];
			config.transitionTextureIndex = 1;	//トランジションテクスチャのインデックスは1で固定
			postEffect->CopyBuffer(PostEffectJob::Dissolve, config);
		}
		ImGui::End();


		config.jobs_ =
			uint32_t(grayScale) << 1 |
			uint32_t(vignette) << 2 |
			uint32_t(boxBlur) << 3 |
			uint32_t(gaussBlur) << 4 |
			uint32_t(edgeDetection) << 5 |
			uint32_t(outline) << 6 |
			uint32_t(radialBlur) << 7 |
			uint32_t(dissolve) << 8;
	}
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

	effect_.Initialize(engine_);

	waveEmitter_ = std::make_unique<WaveEmitter>();
	effect_.AddEmitter(waveEmitter_.get());

	polygonEmitter_ = std::make_unique<PolygonEmitter>();
	effect_.AddEmitter(polygonEmitter_.get());
	

	auto model = modelManager_->GetModelData(SHEngine::TestModel::Cube);	//Cube
	auto ddsTexture = textureManager_->GetTextureData(textureManager_->LoadTexture("rostock_laage_airport_4k.dds"));
	tetris_.Initialize(keyCoating_.get(), worldCamera_, model->meshes.front(), ddsTexture);

	//PostEffectの初期化
	postEffect_.Initialize(textureManager_);		//描画だけするやつなのでコピーオンリー
	postEffectConfig_.origin = commonData_->display.get();
	postEffectConfig_.jobs_ = uint32_t(PostEffectJob::None);

	intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
	intermediateDisplay_->Initialize(1280, 720, "EdgeDetection");
	intermediateDisplay_->AddRenderTarget(textureManager_, 0xff);

	edgeDetection_ = std::make_unique<PostEffect>();
	edgeDetection_->Initialize(textureManager_);
	forEdgeDetection_.origin = commonData_->display.get();
	forEdgeDetection_.output = intermediateDisplay_.get();
	forEdgeDetection_.jobs_ = uint32_t(PostEffectJob::EdgeDetection);

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);

	waveEmitterConfig_.textureID = textureManager_->LoadTexture("MagicCircle.png");

	model = modelManager_->LoadModel("Pyramid");
	auto config = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
	polygonConfigs_.push_back(config);
	auto config2 = polygonEmitter_->AddPolygon(model->meshes, Matrix4x4::Identity(), Vector4(1, 1, 1, 1), 100);
	polygonConfigs_.push_back(config2);

	Load();
}

std::unique_ptr<IScene> GameScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	waveEmitter_->SetConfig(waveEmitterConfig_);
	for (const auto& config : polygonConfigs_) {
		polygonEmitter_->SetConfig(config);
	}
	polygonEmitter_->SetIgnoreBalls(ignoreBalls_);

	computeContext_->BeginTimeStamp("Particle Update");
	effect_.Update(worldCamera_, deltaTime);
	computeContext_->EndTimeStamp();

	input_->Update();
	commonData_->keyManager->Update();
	keyCoating_->Update(deltaTime);

	debugCamera_->Update();
	gameCamera_->Update(deltaTime);

	tetris_.Update(deltaTime);

	//線を消したときのやつ
	int deleteNum = tetris_.IsLineDeleted();
	if (deleteNum) {
		waveEmitter_->AddWave(waves_[deleteNum]);
		if (deleteNum > 2) {
			gameCamera_->Shake(0.3f * deleteNum, 0.5f);
		}
	}
	auto key = keyCoating_->GetKeyStates();

	if (key.at(Key::Debug1)) {
		return std::make_unique<GameScene>();
	}

	if (tetris_.IsGameOver() && (key.at(Key::HardDrop) || key.at(Key::Hold))) {
		return std::make_unique<TitleScene>();
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
	gameCamera_->DrawImGui();
	//tetris_.DrawImGui();
	//timeViewer_->DrawImGui();

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
		ImGui::Begin("IgnoreBall");
		ignoreBalls_.resize(1);
		ImGui::DragFloat3("position", &ignoreBalls_[0].position.x, 0.05f);
		ImGui::DragFloat("radius", &ignoreBalls_[0].radius, 0.01f);
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
}
