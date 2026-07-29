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

	{
		std::string path = "Title";
		testModel_ = std::make_unique<ModelDrawer>();
		testModel_->Initialize(modelManager_->LoadModel(path), "TestModel");
		Transform trans;
		testModel_->SetTransform({ trans.Matrix() });
	}

	Load();

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_);

	peConfig_.dcc = directContext_;
	peConfig_.origin = commonData_->display.get();
	peConfig_.jobs = 0;

	edgeTexture_ = std::make_unique<SHEngine::Screen::Display>();
	edgeTexture_->Initialize(1280, 720, "EdgeTexture");
	edgeTexture_->AddRenderTarget(textureManager_, 0x0000ff);

	edgePostEffect_ = std::make_unique<PostEffect>();
	edgePostEffect_->Initialize(textureManager_);

	edgePeConfig_.dcc = directContext_;
	edgePeConfig_.origin = commonData_->display.get();
	edgePeConfig_.output = edgeTexture_.get();
	edgePeConfig_.jobs = (uint32_t)PostEffectJob::EdgeDetection;


	dissolve_.noiseTextureIndex = textureManager_->LoadTexture("Noise0.png");
	dissolve_.transitionTextureIndex = textureManager_->GetUVChecker();

	EdgeDetection edgeDetection;
	edgeDetection.edgeTextureIndex = commonData_->display->GetTextureData()->GetHandle();
	edgePostEffect_->CopyBuffer(PostEffectJob::EdgeDetection, edgeDetection);

	outline_.edgeTextureIndex = edgeTexture_->GetTextureData()->GetHandle();

	auto plane = modelManager_->GetModelData(SHEngine::TestModel::Plane);
	text_.Initialize(plane->meshes.front(), "ZenOldMincho-Medium.ttf");
	
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update();
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter(), debugCamera_->GetVPMatrix());

	// ===================== 超えられない壁 =================================

	Vector2 mousePos = commonData_->display->GetCursorPos(input_->GetCursorPos());

	testModel_->Update(debugCamera_.get(), deltaTime);

	static float timer = 0.0f;
	timer += deltaTime;

	if (key[Key::One]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::GrayScale;
	} else if (key[Key::Two]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::Vignette;
	} else if (key[Key::Three]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::BoxBlur;
	} else if (key[Key::Four]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::GaussBlur;
	} else if (key[Key::Five]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::EdgeDetection;
	} else if (key[Key::Six]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::Outline;
	} else if (key[Key::Seven]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::RadialBlur;
	} else if (key[Key::Eight]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::Dissolve;
	} else if (key[Key::Nine]) {
		peConfig_.jobs = (uint32_t)PostEffectJob::Fade;
	} else if (key[Key::Zero]) {
		peConfig_.jobs = 0;
	}

	float floatingT = (std::sin(timer) + 1.f) / 2.f;
	dissolve_.threshold = floatingT;
	fade_.t = floatingT;

	postEffect_->CopyBuffer(PostEffectJob::GrayScale, grayScale_);
	postEffect_->CopyBuffer(PostEffectJob::Vignette, vignette_);
	postEffect_->CopyBuffer(PostEffectJob::BoxBlur, boxBlur_);
	postEffect_->CopyBuffer(PostEffectJob::GaussBlur, gaussBlur_);
	postEffect_->CopyBuffer(PostEffectJob::Outline, outline_);
	postEffect_->CopyBuffer(PostEffectJob::RadialBlur, radialBlur_);
	postEffect_->CopyBuffer(PostEffectJob::Dissolve, dissolve_);
	postEffect_->CopyBuffer(PostEffectJob::Fade, fade_);
	postEffect_->CopyBuffer(PostEffectJob::Outline, outline_);
	postEffect_->CopyBuffer(PostEffectJob::RadialBlur, radialBlur_);
	postEffect_->CopyBuffer(PostEffectJob::Dissolve, dissolve_);
	postEffect_->CopyBuffer(PostEffectJob::Fade, fade_);


	int jobIndex = 0;
	if (peConfig_.jobs != 0) {
		jobIndex = std::countr_zero(peConfig_.jobs);
	}
	std::string outputString = "CurrentEffect: " + postEffectNames_[jobIndex];

	text_.SetText(outputString);
	text_.SetColor(textColor_);
	text_.SetTransform(textTransform_);
	text_.Update(orthoCamera_.GetVPMatrix());

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();
	directContext_->SetRenderTarget(display);
	grid_->Draw(directContext_);


	// ↓↓↓ オブジェクト描画 ==============================================

	testModel_->Draw(directContext_);

	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

	edgePostEffect_->Draw(edgePeConfig_);

#ifdef SH_RELEASE
	peConfig_.output = commonData_->window->GetCurrentDisplay();
	postEffect_->Draw(peConfig_);
#else
	postEffect_->Draw(peConfig_);
#endif

#ifdef SH_RELEASE
	directContext_->SetRenderTarget(window, false);
	text_.Draw(directContext_);
#else
	directContext_->SetRenderTarget(display, false);
	text_.Draw(directContext_);
	display->ToTexture(directContext_);
	directContext_->SetRenderTarget(window);
#endif


#ifdef USE_IMGUI

	ImGui::Begin("PostEffect");
	if (ImGui::TreeNode("GrayScale")) {
		ImGui::DragFloat("Intensity", &grayScale_.intensity, 0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Vignette")) {
		ImGui::ColorEdit4("Color", &vignette_.color.x);
		ImGui::DragFloat("Intensity", &vignette_.intensity, 0.01f);
		ImGui::DragFloat("Radius", &vignette_.radius, 0.01f);
		ImGui::DragFloat("Softness", &vignette_.softness, 0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("BoxBlur")) {
		ImGui::DragInt("Kernel Size", (int*)&boxBlur_.kernelSize, 1, 0);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("GaussBlur")) {
		ImGui::DragInt("Kernel Size", (int*)&gaussBlur_.kernelSize, 1, 0);
		ImGui::DragFloat("Sigma", &gaussBlur_.sigma, 0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("EdgeDetection")) {
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Outline")) {
		ImGui::ColorEdit4("Color", &outline_.color.x);
		ImGui::DragFloat("Strength", &outline_.strength, 0.01f, 0);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("RadialBlur")) {
		ImGui::DragFloat2("Center", &radialBlur_.center.x, 0.01f);
		ImGui::DragFloat("Strength", &radialBlur_.strength, 0.01f);
		ImGui::DragInt("Sample Count", &radialBlur_.sampleCount, 0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Dissolve")) {
		ImGui::DragFloat("Threshold", &dissolve_.threshold, 0.01f);
		ImGui::DragFloat("Edge Threshold", &dissolve_.edgeThreshold, 0.01f);
		ImGui::ColorEdit3("Edge Color", &dissolve_.edgeColor.x);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Fade")) {
		ImGui::ColorEdit4("Color", &fade_.color.x);
		ImGui::TreePop();
	}
	ImGui::End();

	ImGui::Begin("Text");
	ImGui::DragFloat3("Scale", &textTransform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &textTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &textTransform_.position.x, 1.f);
	ImGui::ColorEdit4("Color", &textColor_.x);
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

	bin.Register(&grayScale_.intensity);

	bin.Register(&vignette_.intensity);
	bin.Register(&vignette_.radius);

	bin.Register(&boxBlur_.kernelSize);

	bin.Register(&gaussBlur_.kernelSize);
	bin.Register(&gaussBlur_.sigma);

	bin.Register(&outline_.color);
	bin.Register(&outline_.strength);

	bin.Register(&radialBlur_.center);
	bin.Register(&radialBlur_.strength);
	bin.Register(&radialBlur_.sampleCount);

	bin.Register(&dissolve_.noiseTextureIndex);
	bin.Register(&dissolve_.transitionTextureIndex);
	bin.Register(&dissolve_.threshold);
	bin.Register(&dissolve_.edgeThreshold);
	bin.Register(&dissolve_.edgeColor);

	bin.Register(&fade_.color);

	bin.Register(&textTransform_);
	bin.Register(&textColor_);

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = "TestScene_Config.bin";

	if (!bin.Boot(fileName)) {
		return;
	}

	grayScale_.intensity = bin.Reverse<float>();

	vignette_.intensity = bin.Reverse<float>();
	vignette_.radius = bin.Reverse<float>();

	boxBlur_.kernelSize = bin.Reverse<uint32_t>();

	gaussBlur_.kernelSize = bin.Reverse<uint32_t>();
	gaussBlur_.sigma = bin.Reverse<float>();

	outline_.color = bin.Reverse<Vector4>();
	outline_.strength = bin.Reverse<float>();

	radialBlur_.center = bin.Reverse<Vector2>();
	radialBlur_.strength = bin.Reverse<float>();
	radialBlur_.sampleCount = bin.Reverse<int>();

	dissolve_.noiseTextureIndex = bin.Reverse<uint32_t>();
	dissolve_.transitionTextureIndex = bin.Reverse<uint32_t>();
	dissolve_.threshold = bin.Reverse<float>();
	dissolve_.edgeThreshold = bin.Reverse<float>();
	dissolve_.edgeColor = bin.Reverse<Vector3>();

	fade_.color = bin.Reverse<Vector4>();


	textTransform_ = bin.Reverse<Transform>();
	textColor_ = bin.Reverse<Vector4>();
}
