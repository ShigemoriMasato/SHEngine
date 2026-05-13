#include "DDSTest.h"

void DDSTest::Initialize(SHEngine::Engine* engine) {
	//DDSを読み込む
	auto tm = engine->GetTextureManager();
	auto ddsData = tm->GetTextureData(tm->LoadTexture("rostock_laage_airport_4k.dds"));

	//描画する頂点などの取得
	auto ddm = engine->GetDrawDataManager();
	auto mm = engine->GetModelManager();
	auto drawData = ddm->GetDrawData(mm->GetNodeModelData(0).drawDataIndex);

	//CPUの情報をShaderに送信する用のBufferを作成する
	container_ = std::make_unique<SHEngine::BufferContainer>();
	auto ddsBuffer = container_->Create(ddsData);
	wvpBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));

	//Rendererを作成
	ddsCube_ = std::make_unique<SHEngine::Renderer>(drawData);
	ddsCube_->SetVS("Test/DDSTest.VS.hlsl");
	ddsCube_->SetPS("Test/DDSTest.PS.hlsl");
	ddsCube_->SetGPUBuffer(wvpBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	ddsCube_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::DDSTexture);
}

void DDSTest::Update(float deltaTime, Camera* camera) {
#ifdef USE_IMGUI
	ImGui::Begin("DDS Cube");
	ImGui::DragFloat3("Scale", &scale_.x, 1.0f, 1.0f);
	ImGui::End();
#endif

	Matrix4x4 wvp = Matrix::MakeScaleMatrix(scale_) * camera->GetVPMatrix();
	wvpBuffer_->CopyBuffer(&wvp, sizeof(wvp));
}

void DDSTest::Draw(CmdObj* cmdObj) {
	ddsCube_->Draw(cmdObj);
}
