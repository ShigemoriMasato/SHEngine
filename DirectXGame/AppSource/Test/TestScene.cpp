#include "TestScene.h"
#include <Render/Buffer/ResourceRegistry.h>
#include <Compute/Graph/ComputeGraph.h>

TestScene::~TestScene() {
	Save();
}

void TestScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(input_);

	gameCamera_ = std::make_unique<Camera>();
	gameCamera_->SetProjectionMatrix(PerspectiveFovDesc());

	orthoCamera_.SetProjectionMatrix(OrthographicDesc());
	orthoCamera_.MakeMatrix();

	grid_ = std::make_unique<Grid>();
	grid_->Initialize();

	// ===================== 超えられない壁 =================================

	curveBinders_.resize(2);
	curveBinders_[0].BindVector3("Test1", &testParam_);
	curveBinders_[1].BindFloat("Test2", &testParam2_);

	curveEditor_.SetBinder(&curveBinders_[0]);
	curveEditor_.SetBinder(&curveBinders_[1]);

	container_ = std::make_unique<SHEngine::BufferContainer>();

	particleSystem_.Initialize(computeContext_);
	particleSystem_.SetCamera(debugCamera_.get());
	emitter_ = particleSystem_.CreateEmitter(1024 * 1024, computeContext_);

	auto model = modelManager_->GetModelData("Pyramid");
	Particle::InnerModel::EmitInnerModel(emitter_, container_.get(), 1024 * 1024, model, 0.1f, {0.3f, 0.3f, 0.3f, 0.3f});

	InitializeGraphTest();
}

std::unique_ptr<IScene> TestScene::Update() {
	debugCamera_->Update(commonData_->display->IsForcus());

	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();
	float deltaTime = engine_->GetDeltaTime();

	grid_->Update(debugCamera_->GetCenter());

	particleSystem_.Update(computeContext_);

	UpdateGraphTest();

	// ===================== 超えられない壁 =================================

	commonData_->display->DrawImGui();
	ImGuizmo::OriginalSetRect(commonData_->display.get());

	// ===================== 超えられない壁 =================================

	curveEditor_.Update(deltaTime);

	curveEditor_.DrawWindow();

	return nullptr;
}

void TestScene::Draw() {
	auto window = commonData_->window.get();
	auto display = commonData_->display.get();

	{
		auto fence = computeContext_->MiddleExecute();
		directContext_->WaitFenceInCPU(fence);

		directContext_->SetRenderTarget(display);
		particleSystem_.Draw(directContext_);

		fence = directContext_->MiddleExecute();
		computeContext_->WaitFenceInCPU(fence);
	}

	directContext_->SetRenderTarget(display, false);

	grid_->SetCamera(debugCamera_.get());
	grid_->Draw(directContext_);

	// ↓↓↓ オブジェクト描画 ==============================================



	// ↑↑↑ オブジェクト描画 ==============================================

	display->ToTexture(directContext_);

	directContext_->SetRenderTarget(window);
	// ComputeShaderの実行順・割り当て。選択モードを開始するのでResourceRegistryより先に呼ぶ
	SHEngine::ComputeGraphRegistry::DrawImGui();
	// GPUBuffer・テクスチャの一覧
	SHEngine::ResourceRegistry::DrawImGui();
	engine_->DrawImGui();

	window->ToPresent(directContext_);
}

void TestScene::Save() {
	BinaryManager bin;
	const std::string fileName = basePath_ + currentFileName_ + extension_;

	// ↓↓↓ 保存するデータ ==============================================

	// ↑↑↑ 保存するデータ ==============================================

	bin.Write(fileName);
}

void TestScene::Load() {
	BinaryManager bin;
	const std::string fileName = basePath_ + currentFileName_ + extension_;

	if (!bin.Boot(fileName)) {
		return;
	}

}

// ===================== ComputeGraphのテスト =================================

void TestScene::InitializeGraphTest() {
	using namespace SHEngine;

	// Resourceウィンドウでは "GraphTest" グループに出てくる
	graphTestContainer_ = std::make_unique<BufferContainer>(8, "GraphTest");
	fillConfigBuffer_ = graphTestContainer_->Create(BufferType::CBV, sizeof(FillConfig), 1, BufferNum::MatchSwapChain, "FillConfig");
	addConfigBuffer_ = graphTestContainer_->Create(BufferType::CBV, sizeof(AddConfig), 1, BufferNum::MatchSwapChain, "AddConfig");
	inputBuffer_ = graphTestContainer_->Create(BufferType::SRV, sizeof(float), kGraphTestCount, BufferNum::MatchSwapChain, "Input");
	// UAVは毎フレーム同じリソースを使うようにSingleにする
	outputBuffer_ = graphTestContainer_->Create(BufferType::SRV_UAV, sizeof(float), kGraphTestCount, BufferNum::Single, "Output");
	counterBuffer_ = graphTestContainer_->Create(BufferType::UAV, sizeof(uint32_t), kGraphTestCount, BufferNum::Single, "Counter");
	pairBuffer_ = graphTestContainer_->Create(BufferType::UAV, sizeof(float) * 2, kGraphTestCount, BufferNum::Single, "Pair(stride8)");

	float input[kGraphTestCount];
	for (uint32_t i = 0; i < kGraphTestCount; ++i) {
		input[i] = float(i);
	}
	inputBuffer_->CopyBuffer(input, sizeof(input));

	outputReadBack_ = std::make_unique<ReadBackBuffer>(sizeof(float) * kGraphTestCount);

	// グラフは "Assets/Binary/ComputeGraph/Test/GraphTest.bin" に保存される
	graphTrigger_ = std::make_unique<ComputeTrigger>("Test/GraphTest");
	graphTrigger_->Bind("Output", outputBuffer_);
	graphTrigger_->Bind("Counter", counterBuffer_);
	graphTrigger_->SetCount("Count", kGraphTestCount);
}

void TestScene::UpdateGraphTest() {
	// 前回までの結果(数フレーム前にコピーしたもの)
	std::memcpy(outputValues_, outputReadBack_->GetData(computeContext_), sizeof(outputValues_));

#ifdef USE_IMGUI
	ImGui::Begin("ComputeGraphTest");
	ImGui::TextDisabled("Trigger: \"%s\"", graphTrigger_->GetName().c_str());
	if (ImGui::Button("見本グラフを作る (Fill -> Add)")) {
		BuildSampleGraph();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("今あるノードを消して、Fill → Add の順に実行するグラフを作ります。\n保存はComputeGraphウィンドウの「保存」で行います。");
	}
	ImGui::Checkbox("毎フレームFire", &fireEveryFrame_);
	ImGui::SameLine();
	bool fireOnce = ImGui::Button("1回Fire");
	ImGui::Checkbox("同一フレームで2回Fire (CBV書き換え警告の確認)", &fireTwice_);

	ImGui::SeparatorText("FillConfig (b0)");
	ImGui::DragFloat("scale", &fillConfig_.scale, 0.1f);
	ImGui::DragFloat("offset", &fillConfig_.offset, 0.1f);
	int count = int(fillConfig_.count);
	if (ImGui::SliderInt("count", &count, 0, int(kGraphTestCount))) {
		fillConfig_.count = uint32_t(count);
	}
	ImGui::SeparatorText("AddConfig (b0)");
	ImGui::DragFloat("add", &addConfig_.add, 1.0f);

	ImGui::SeparatorText("Output (ReadBack)");
	ImGui::TextDisabled("Fill: input[i] * scale + offset → Add: + add");
	if (ImGui::BeginTable("Output", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
		for (uint32_t i = 0; i < 32; ++i) {
			ImGui::TableNextColumn();
			ImGui::Text("%.1f", outputValues_[i]);
		}
		ImGui::EndTable();
	}
	ImGui::End();
#else
	bool fireOnce = false;
#endif

	fillConfigBuffer_->CopyBuffer(&fillConfig_, sizeof(fillConfig_));
	addConfigBuffer_->CopyBuffer(&addConfig_, sizeof(addConfig_));

	if (!fireEveryFrame_ && !fireOnce) {
		return;
	}

	graphTrigger_->Fire(computeContext_);
	if (fireTwice_) {
		// Fire(使用)した後に同じフレームで書き換える → 警告が出る(GPU上では両方とも後の値になる)
		FillConfig second = fillConfig_;
		second.offset += 1000.0f;
		fillConfigBuffer_->CopyBuffer(&second, sizeof(second));
		graphTrigger_->Fire(computeContext_);
	}

	outputReadBack_->Copy(outputBuffer_, computeContext_);
}

void TestScene::BuildSampleGraph() {
	using namespace SHEngine;

	ComputeGraph* graph = graphTrigger_->GetGraph();

	// Trigger以外を消す
	std::vector<GraphNodeID> removeIDs;
	for (const auto& [id, node] : graph->GetNodes()) {
		if (node.kind != GraphNodeKind::Trigger) removeIDs.push_back(id);
	}
	for (GraphNodeID id : removeIDs) {
		graph->RemoveNode(id);
	}

	auto addResource = [&](GPUBuffer* buffer, const Vector2& position) -> GraphNodeID {
		GraphNode& node = graph->AddNode(GraphNodeKind::Resource, position);
		node.resource = ResourceRef::Buffer(graphTestContainer_->GetResourceID(buffer));
		node.resourceName = ResourceRegistry::GetPersistentName(node.resource);
		return node.id;
		};

	// Fill: 数はコードからSetCountした "Count"
	GraphNodeID fill = graph->AddNode(GraphNodeKind::Dispatch, { 320.0f, 60.0f }).id;
	{
		GraphNode& node = *graph->GetNode(fill);
		node.shaderName = "Test/GraphTest/Fill.CS.hlsl";
		node.dispatchMode = DispatchMode::ArgumentCount;
		node.countArgument = "Count";
	}
	// Add: 数はu0につながったバッファの要素数
	GraphNodeID add = graph->AddNode(GraphNodeKind::Dispatch, { 680.0f, 60.0f }).id;
	{
		GraphNode& node = *graph->GetNode(add);
		node.shaderName = "Test/GraphTest/Add.CS.hlsl";
		node.dispatchMode = DispatchMode::ResourceCount;
		node.countPin = { SlotRequest::Type::UAV, 0, "output" };
	}

	GraphNodeID fillConfig = addResource(fillConfigBuffer_, { 40.0f, 200.0f });
	GraphNodeID input = addResource(inputBuffer_, { 40.0f, 290.0f });
	GraphNodeID addConfig = addResource(addConfigBuffer_, { 420.0f, 300.0f });
	GraphNodeID output = graph->AddNode(GraphNodeKind::Argument, { 40.0f, 380.0f }).id;
	graph->GetNode(output)->argumentName = "Output";

	graph->SetNext(graph->GetTriggerNode()->id, fill);
	graph->SetNext(fill, add);

	graph->Connect(fillConfig, fill, { SlotRequest::Type::CBV, 0, "FillConfig" });
	graph->Connect(input, fill, { SlotRequest::Type::SRV, 0, "input" });
	graph->Connect(output, fill, { SlotRequest::Type::UAV, 0, "output" });
	graph->Connect(addConfig, add, { SlotRequest::Type::CBV, 0, "AddConfig" });
	graph->Connect(output, add, { SlotRequest::Type::UAV, 0, "output" });

	graph->RefreshAllPins();
}
