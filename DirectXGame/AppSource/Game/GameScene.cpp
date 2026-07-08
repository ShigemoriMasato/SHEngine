#include "GameScene.h"
#include <imgui/imgui.h>
#include <Utility/Color.h>
#include <Title/TitleScene.h>

using namespace SHEngine;

namespace {
	Matrix4x4 gameoverMat;
}

GameScene::GameScene() {
	tetris_ = std::make_unique<Tetris>();
	debugCamera_ = std::make_unique<DebugCamera>();
	manualCamera_ = std::make_unique<Camera>();
	manualCamera_->SetProjectionMatrix(PerspectiveFovDesc());
	manualCamera_->MakeMatrix();
	postEffect_ = std::make_unique<PostEffect>();
	gameCamera_ = std::make_unique<GameCamera>();
	effect_ = std::make_unique<Effect>();

	gameCamera_->Initialize();
	worldCamera_ = debugCamera_.get();
}

void GameScene::Initialize() {
	keyCoating_ = std::make_unique<KeyCoating>(commonData_->keyManager.get());
	debugCamera_->Initialize(input_);
	auto model = modelManager_->GetNodeModelData(0);	//Cube
	DrawData drawData = drawDataManager_->GetDrawData(model.drawDataIndex);

	auto ddsTexture = textureManager_->GetTextureData(textureManager_->LoadTexture("rostock_laage_airport_4k.dds"));
	tetris_->Initialize(keyCoating_.get(), worldCamera_, drawData, ddsTexture);

	//PostEffectの初期化
	auto pedd = drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex);
	postEffect_->Initialize(textureManager_, pedd);		//描画だけするやつなのでコピーオンリー
	postEffectConfig_.origin = commonData_->display.get();
	postEffectConfig_.jobs_ = uint32_t(PostEffectJob::None);

	intermediateDisplay_ = std::make_unique<SHEngine::Screen::Display>();
	intermediateDisplay_->Initialize(1280, 720, "EdgeDetection");
	intermediateDisplay_->AddRenderTarget(textureManager_, 0xff);

	edgeDetection_ = std::make_unique<PostEffect>();
	edgeDetection_->Initialize(textureManager_, pedd);
	forEdgeDetection_.origin = commonData_->display.get();
	forEdgeDetection_.output = intermediateDisplay_.get();
	forEdgeDetection_.jobs_ = uint32_t(PostEffectJob::EdgeDetection);

	gameOverText = std::make_unique<RenderObject>("GameOverText");
	gameOverText->Initialize();
	gameOverText->CreateCBV(sizeof(Matrix4x4), ShaderType::VERTEX_SHADER, "GameOverText::WVPMatrix");
	gameOverText->psoConfig_.vs = "Simple.VS.hlsl";
	gameOverText->psoConfig_.ps = "White.PS.hlsl";
	int modelIndex = modelManager_->LoadModel("GameOver");
	model = modelManager_->GetNodeModelData(modelIndex);
	auto gameOverdd = drawDataManager_->GetDrawData(model.drawDataIndex);
	gameOverText->SetDrawData(gameOverdd);

	effect_->Initialize(engine_);

	gameoverMat = Matrix::MakeAffineMatrix(
		Vector3(1.0f, 1.0f, 1.0f),
		Vector3(),
		Vector3(0.0f, 0.0f, -5.0f)
	);

	subject_ = std::make_unique<Subject>();
	subject_->Initialize(engine_);

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);
}

std::unique_ptr<IScene> GameScene::Update() {
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	effect_->Update(worldCamera_->GetVPMatrix(), worldCamera_->GetBillboardMatrix(), deltaTime);

	subject_->Update(worldCamera_->GetVPMatrix());

	input_->Update();
	commonData_->keyManager->Update();
	keyCoating_->Update(deltaTime);

	debugCamera_->Update();
	gameCamera_->Update(deltaTime);

	tetris_->Update(deltaTime);

	//線を消したときのやつ
	int deleteNum = tetris_->IsLineDeleted();
	if (deleteNum) {

		if (deleteNum > 2) {
			gameCamera_->Shake(0.3f * deleteNum, 0.5f);
		}
	}
	auto key = keyCoating_->GetKeyStates();

	if (key.at(Key::Debug1)) {
		return std::make_unique<GameScene>();
	}

	if (tetris_->IsGameOver() && (key.at(Key::HardDrop) || key.at(Key::Hold))) {
		return std::make_unique<TitleScene>();
	}

	return nullptr;
}

void GameScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display, true);

	tetris_->Draw(directContext_);
	if (tetris_->IsGameOver()) {
		Matrix4x4 wvp = gameoverMat * worldCamera_->GetVPMatrix();
		gameOverText->CopyBufferData(0, &wvp, sizeof(Matrix4x4));
		//gameOverText->Draw(cmdObj);
	}

	subject_->Draw(directContext_);

	//一番最後に描画
	directContext_->BeginTimeStamp("Particle Draw");
	effect_->Draw();
	directContext_->EndTimeStamp();

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
	postEffect_->Draw(postEffectConfig_);
	directContext_->SetRenderTarget(window, false);

#else

	postEffect_->Draw(postEffectConfig_);
	directContext_->SetRenderTarget(window);

#endif

	//ここ以外で記述する場合、ifdefを忘れないようにすること
#ifdef USE_IMGUI
	display->DrawImGui();
	manualCamera_->DrawImGui();
	manualCamera_->MakeMatrix();
	tetris_->DrawImGui();
	gameCamera_->DrawImGui();
	timeViewer_->DrawImGui();
	auto depthPtr = commonData_->display->GetDepthTexture()->GetGPUHandle().ptr;
	ImGui::Begin("Depth");
	ImGui::Image(ImTextureRef(depthPtr), ImVec2(1280 / 4, 720 / 4));
	ImGui::End();

	ImGui::Begin("FPS");
	float deltaTime = engine_->GetFPSObserver()->GetDeltatime();
	float fps = 1.0f / deltaTime;
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("DeltaTime: %.4f sec", deltaTime);
	ImGui::End();

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
		postEffect_->CopyBuffer(PostEffectJob::GrayScale, config);
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
		postEffect_->CopyBuffer(PostEffectJob::Vignette, config);
	}
	ImGui::Checkbox("BoxBlur", &boxBlur);
	if (boxBlur) {
		static Blur config;
		ImGui::PushID("BoxBlur");
		ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 30);
		ImGui::PopID();
		postEffect_->CopyBuffer(PostEffectJob::BoxBlur, config);
	}
	ImGui::Checkbox("GaussBlur", &gaussBlur);
	if (gaussBlur) {
		static GaussBlur config;
		ImGui::PushID("GaussBlur");
		ImGui::SliderInt("KernelSize", reinterpret_cast<int*>(&config.kernelSize), 1, 15);
		ImGui::DragFloat("Sigma", &config.sigma, 0.01f);
		ImGui::PopID();
		postEffect_->CopyBuffer(PostEffectJob::GaussBlur, config);
	}
	ImGui::Checkbox("EdgeDetection", &edgeDetection);
	if (edgeDetection) {
		//データは必要ないため無記入
	}
	ImGui::Checkbox("Outline", &outline);
	if (outline) {
		static Outline config;
		config.edgeTextureIndex = intermediateDisplay_->GetTextureData()->GetHandle();
		ImGui::PushID("Outline");
		ImGui::ColorEdit4("Color", &config.color.x);
		ImGui::DragFloat("Strength", &config.strength, 0.01f);
		ImGui::PopID();
		postEffect_->CopyBuffer(PostEffectJob::Outline, config);
	}
	ImGui::Checkbox("RadialBlur", &radialBlur);
	if (radialBlur) {
		static RadialBlur config;
		ImGui::PushID("RadialBlur");
		ImGui::DragFloat2("Center", &config.center.x, 0.01f);
		ImGui::DragFloat("Strength", &config.strength, 0.01f);
		ImGui::DragInt("SampleCount", &config.sampleCount, 1, 1, 10);
		ImGui::PopID();
		postEffect_->CopyBuffer(PostEffectJob::RadialBlur, config);
	}
	ImGui::Checkbox("Dissolve", &dissolve);
	if (dissolve) {
		static std::vector<int> noise = {
			textureManager_->LoadTexture("Noise0.png"),
			textureManager_->LoadTexture("Noise1.png")
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
		postEffect_->CopyBuffer(PostEffectJob::Dissolve, config);
	}
	ImGui::End();
	

	postEffectConfig_.jobs_ =
		uint32_t(grayScale) << 1 |
		uint32_t(vignette) << 2 |
		uint32_t(boxBlur) << 3 |
		uint32_t(gaussBlur) << 4 |
		uint32_t(edgeDetection) << 5 |
		uint32_t(outline) << 6 | 
		uint32_t(radialBlur) << 7 |
		uint32_t(dissolve) << 8;
#endif

	engine_->DrawImGui();
	window->ToPresent(directContext_);
}
