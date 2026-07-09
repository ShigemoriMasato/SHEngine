#include "EffectTestScene.h"

void EffectTestScene::Initialize() {
	camera_.Initialize(input_);

	effect_ = std::make_unique<Effect>();
	effect_->Initialize(engine_);

	grid_ = std::make_unique<Grid>();
	grid_->Initialize(drawDataManager_);

	vertexEmitter_ = std::make_unique<VertexEmitter>();
	effect_->AddEmitter(vertexEmitter_.get());

	auto model = modelManager_->GetNodeModelData(modelManager_->LoadModel("WaterPlane"));
	vertexEmitter_->AddModel(model.positions, Vector4(1.0f, 1.0f, 1.0f, 1.0f), computeContext_);

	readBackBuffer_ = std::make_unique<SHEngine::ReadBackBuffer>(sizeof(uint32_t) * model.positions.size());

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex), true);
	peConfig_.origin = commonData_->display.get();
	peConfig_.output = commonData_->window.get();
	peConfig_.jobs_ = uint32_t(PostEffectJob::None);
	peConfig_.dcc = directContext_;

	timeViewer_ = std::make_unique<TimeViewer>();
	timeViewer_->Initialize(engine_);
}

std::unique_ptr<IScene> EffectTestScene::Update() {
	camera_.Update();

	grid_->Update(camera_.GetCenter(), camera_.GetVPMatrix());

	static auto model = modelManager_->GetNodeModelData(modelManager_->LoadModel("WaterPlane"));
	vertexEmitter_->EditPosition(0, model.positions, computeContext_);
	vertexEmitter_->EditColor(0, Vector4(1.0f, 1.0f, 1.0f, 1.0f), computeContext_);

	vertexEmitter_->CopyIndexList(0, readBackBuffer_.get(), computeContext_);

	effect_->Update(camera_.GetVPMatrix(), camera_.GetBillboardMatrix(), engine_->GetDeltaTime());

	void* rawdata = readBackBuffer_->GetData(computeContext_);

	std::vector<uint32_t> indices(model.positions.size());
	std::memcpy(indices.data(), rawdata, sizeof(uint32_t) * model.positions.size());

	if (indices[1] > 0) {
		static bool isFirst = true;
		if (isFirst) {
			isFirst = false;
			static Logger logger = GetLogger("IndexList");
			std::string log = "";
			for (const auto& i : indices) {
				log += std::to_string(i) + ",";
			}
			logger->info("Indices: {}", log.c_str());
		}
	}

	return std::unique_ptr<IScene>();
}

void EffectTestScene::Draw() {
	auto display = commonData_->display.get();
	auto window = commonData_->window.get();

	directContext_->SetRenderTarget(display);

	//grid_->Draw(directContext_);

	directContext_->BeginTimeStamp("Particle Draw");
	effect_->Draw();
	directContext_->EndTimeStamp();

	timeViewer_->Add("Particle Update", engine_->GetComputeCommandContext()->GetTimeStampResult("Particle Update"));
	timeViewer_->Add("Particle Draw", directContext_->GetTimeStampResult("Particle Draw"));
	timeViewer_->Add("DeltaTime", engine_->GetDeltaTime());
	timeViewer_->Draw(directContext_);

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
