#include "EffectTestScene.h"

namespace {
	std::string filePath = "WaterPlane";
}

void EffectTestScene::Initialize() {
	camera_.Initialize(input_);

	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto pedd = drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex);
	effect_ = std::make_unique<Effect>();
	effect_->Initialize(engine_, commonData_->display.get(), pedd);

	grid_ = std::make_unique<Grid>();
	grid_->Initialize(drawDataManager_);

	vertexEmitter_ = std::make_unique<VertexEmitter>();
	effect_->AddEmitter(vertexEmitter_.get());

	auto model = modelManager_->GetNodeModelData(modelManager_->LoadModel(filePath));
	//vertexEmitter_->AddModel(model.positions, Vector4(1.0f, 1.0f, 1.0f, 1.0f), computeContext_);

	polygonEmitter_ = std::make_unique<PolygonEmitter>();
	effect_->AddEmitter(polygonEmitter_.get());

	auto polygonList = modelManager_->CreatePolygonList(modelManager_->LoadModel(filePath));
	polygonEmitter_->AddPolygon(polygonList, Matrix::MakeTranslationMatrix({0, 0, 0}), Vector4(1.0f, 1.0f, 1.0f, 1.0f), 100);

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

	computeContext_->BeginTimeStamp("Particle Update");

	grid_->Update(camera_.GetCenter(), camera_.GetVPMatrix());

	static auto model = modelManager_->GetNodeModelData(modelManager_->LoadModel(filePath));

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

	effect_->Draw();
	
	directContext_->SetRenderTarget(display, false);

	//grid_->Draw(directContext_);

	timeViewer_->Add("Particle Update", engine_->GetComputeCommandContext()->GetTimeStampResult("Particle Update"));
	//timeViewer_->Add("Particle Draw", directContext_->GetTimeStampResult("Particle Draw"));
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
