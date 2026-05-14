#include "Subject.h"

void Subject::Initialize(SHEngine::Engine* engine) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto tm = engine->GetTextureManager();
	auto mm = engine->GetModelManager();
	auto ddm = engine->GetDrawDataManager();

	auto ddsData = tm->GetTextureData(tm->LoadTexture("rostock_laage_airport_4k.dds"));

	auto ddsBuffer = container_->Create(ddsData);
	wvp_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));

	auto drawData = ddm->GetDrawData(mm->GetNodeModelData(mm->LoadModel("Innenr_Cube")).drawDataIndex);

	cube_ = std::make_unique<SHEngine::Renderer>(drawData);
	cube_->SetVS("Test/DDS/Cube.VS.hlsl");
	cube_->SetPS("Test/DDS/Cube.PS.hlsl");
	cube_->SetGPUBuffer(wvp_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	cube_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::DDSTexture);
}

void Subject::Update(const Matrix4x4& vpMat) {
	Matrix4x4 wvp = Matrix::MakeScaleMatrix({512, 512, 512}) * vpMat;
	wvp_->CopyBuffer(&wvp, sizeof(wvp));
}

void Subject::Draw(CmdObj* cmdObj) {
	cube_->Draw(cmdObj);
}
