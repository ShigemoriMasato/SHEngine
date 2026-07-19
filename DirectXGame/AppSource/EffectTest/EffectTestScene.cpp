#include "EffectTestScene.h"

namespace {
	std::string filePath = "Title";
}

void EffectTestScene::Initialize() {
	camera_.Initialize(input_);

	container_ = std::make_unique<SHEngine::BufferContainer>();

	effect_ = std::make_unique<Effect>();
	effect_->Initialize(engine_);

	grid_ = std::make_unique<Grid>();
	grid_->Initialize();

	vertexEmitter_ = std::make_unique<VertexEmitter>();
	effect_->AddEmitter(vertexEmitter_.get());
	auto model = modelManager_->LoadModel(filePath);
	vertexEmitter_->AddModel(model->meshes.front().position, Vector4(1.0f, 1.0f, 1.0f, 1.0f), computeContext_);

	polygonEmitter_ = std::make_unique<PolygonEmitter>(65535 * 128 - 1);
	effect_->AddEmitter(polygonEmitter_.get());
	
	waveEmitter_ = std::make_unique<WaveEmitter>(65535 * 128 - 1);
	effect_->AddEmitter(waveEmitter_.get());

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex), true);
	peConfig_.origin = commonData_->display.get();
	peConfig_.output = commonData_->window.get();
	peConfig_.jobs_ = uint32_t(PostEffectJob::None);
	peConfig_.dcc = directContext_;

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);


	unorderedTest_ = std::make_unique<SHEngine::Screen::Display>();
	unorderedTest_->Initialize(1280, 720, "Unordered Test");
	unorderedTest_->AddRenderTarget(textureManager_, 0x000000ff, SHEngine::Format::R8G8B8A8_UNORM, true);

	auto unorderedTestTexture = unorderedTest_->GetTextureData();
	auto utBuffer = container_->Create(unorderedTestTexture);

	unorderedTestCom_ = std::make_unique<SHEngine::ComputeObject>();
	unorderedTestCom_->SetShader("Test/UnorderedTest.CS.hlsl");
	unorderedTestCom_->SetThreadGroupSize(1280 * 720 / 128 + 1, 1, 1);
	unorderedTestCom_->SetGPUBuffer(BufferType::UAV, utBuffer);
}

std::unique_ptr<IScene> EffectTestScene::Update() {
	camera_.Update();

	waveEmitterConfig_.DrawImGui();
	waveEmitter_->SetConfig(waveEmitterConfig_);

	if (waveData_.DrawImGui()) {
		waveEmitter_->AddWave(waveData_);
	}

	waveEmitter_->DrawImGui();

	computeContext_->BeginTimeStamp("Particle Update");

	grid_->Update(camera_.GetCenter(), camera_.GetVPMatrix());

	static auto model = modelManager_->LoadModel(filePath);

	effect_->Update(camera_.GetVPMatrix(), camera_.GetBillboardMatrix(), engine_->GetDeltaTime());

	computeContext_->EndTimeStamp();

	unorderedTest_->ToUnordered(directContext_, true);
	unorderedTestCom_->Execute(directContext_);
	unorderedTest_->ToTexture(directContext_);

	return std::unique_ptr<IScene>();
}

void EffectTestScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	directContext_->BeginTimeStamp("Particle Draw");
	effect_->Draw();
	directContext_->EndTimeStamp();
	
	directContext_->SetRenderTarget(display, false);

	//grid_->Draw(directContext_);

	timeViewer_->Add("Particle Update", engine_->GetComputeCommandContext()->GetTimeStampResult("Particle Update"));
	timeViewer_->Add("Particle Draw", directContext_->GetTimeStampResult("Particle Draw"));
	timeViewer_->Add("DeltaTime", engine_->GetDeltaTime());
	timeViewer_->Draw(directContext_);

	display->ToNonPixel(directContext_);

	display->ToTexture(directContext_);

	display->DrawImGui();
	timeViewer_->DrawImGui();

	bool isFill = true;
#ifdef SH_RELEASE

	postEffect_->Draw(peConfig_);
	isFill = false;

#endif

	directContext_->SetRenderTarget(window, isFill);

	engine_->DrawImGui();
	window->ToPresent(directContext_);
}

void EffectTestScene::Save() {
	BinaryManager binaryManager;
	binaryManager.Register(&drawGrid_);
	waveEmitterConfig_.Save(binaryManager);
	waveData_.Save(binaryManager);
	binaryManager.Write(savefile_);
}

void EffectTestScene::Load() {
	BinaryManager binaryManager;
	if (!binaryManager.Boot(savefile_)) {
		return;
	}

	drawGrid_ = binaryManager.Reverse<bool>();
	waveEmitterConfig_.Load(binaryManager);
	waveData_.Load(binaryManager);
}
