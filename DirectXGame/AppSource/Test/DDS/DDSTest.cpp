#include "DDSTest.h"
#include <numbers>
#include <Utility/Easing.h>

void DDSTest::Initialize(SHEngine::Engine* engine) {
	//各種マネージャの取得
	auto tm = engine->GetTextureManager();
	auto ddm = engine->GetDrawDataManager();
	auto mm = engine->GetModelManager();

	//DDSを読み込む
	auto ddsData = tm->GetTextureData(tm->LoadTexture("rostock_laage_airport_4k.dds"));
	container_ = std::make_unique<SHEngine::BufferContainer>();
	auto ddsBuffer = container_->Create(ddsData);

	//DDSCubeの描画準備
	{
		//描画する頂点などの取得
		auto drawData = ddm->GetDrawData(mm->GetNodeModelData(0).drawDataIndex);

		//CPUの情報をShaderに送信する用のBufferを作成する
		wvpCubeBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));

		//Rendererを作成
		ddsCube_ = std::make_unique<SHEngine::Renderer>(drawData);
		ddsCube_->SetVS("Test/DDS/Cube.VS.hlsl");
		ddsCube_->SetPS("Test/DDS/Cube.PS.hlsl");
		ddsCube_->SetGPUBuffer(wvpCubeBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
		ddsCube_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::DDSTexture);

		scale_ = { -512, 512, 512 };
	}

	//DDSが映りこむオブジェクトの描画準備
	{
		//描画する頂点などの取得
		auto drawData = ddm->GetDrawData(mm->GetNodeModelData(2).drawDataIndex);

		//CPUの情報をShaderに送信する用のBufferを作成する
		vsBuffer_ = container_->Create(BufferType::CBV, sizeof(VSData));
		psBuffer_ = container_->Create(BufferType::CBV, sizeof(PSData));

		//Rendererの作成
		reflectObj_ = std::make_unique<SHEngine::Renderer>(drawData);
		reflectObj_->SetVS("Test/DDS/Reflect.VS.hlsl");
		reflectObj_->SetPS("Test/DDS/Reflect.PS.hlsl");
		reflectObj_->SetGPUBuffer(vsBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
		reflectObj_->SetGPUBuffer(psBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
		reflectObj_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::DDSTexture);
	}
}

void DDSTest::Update(float deltaTime, Camera* camera) {
#ifdef USE_IMGUI
	ImGui::Begin("DDS Cube");
	ImGui::DragFloat3("Scale", &scale_.x, 1.0f, 1.0f);
	ImGui::End();

	ImGui::Begin("DDS Reflect Object");
	ImGui::DragFloat("Strength", &psData_.strength, 0.01f, 0.0f, 1.0f);
	ImGui::Text("CameraPos: %f, %f, %f", psData_.cameraPos.x, psData_.cameraPos.y, psData_.cameraPos.z);
	ImGui::End();
#endif

	Matrix4x4 wvp = Matrix::MakeScaleMatrix(scale_) * camera->GetVPMatrix();
	wvpCubeBuffer_->CopyBuffer(&wvp, sizeof(wvp));

	timer_ += deltaTime;
	float t = std::fmodf(timer_, 1.0f);
	//position_.y = lerp_RoundTrip(3.0f, -3.0f, t, EaseType::EaseInOutCubic, EaseType::EaseInOutCubic);
	//rotate_.y += std::numbers::pi_v<float> *deltaTime;

	vsData_.world = Matrix::MakeAffineMatrix({ 4.0f, 4.0f, 4.0f }, rotate_, position_);
	vsData_.wvp = vsData_.world * camera->GetVPMatrix();

	psData_.cameraPos = camera->GetPosition();

	vsBuffer_->CopyBuffer(&vsData_, sizeof(vsData_));
	psBuffer_->CopyBuffer(&psData_, sizeof(psData_));
}

void DDSTest::Draw(CmdObj* cmdObj) {
	ddsCube_->Draw(cmdObj);
	reflectObj_->Draw(cmdObj);
}
