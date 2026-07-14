#include "Subject.h"

void Subject::Initialize(SHEngine::Engine* engine) {
	engine_ = engine;

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
	cube_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::SRV);

	animation_ = std::make_unique<Animation_Sub>();
	animation_->Initialize(engine_);

	hitEffect_ = std::make_unique<HitEffect>();
	hitEffect_->Initialize(engine_);

	cylinder_ = std::make_unique<Cylinder>(engine_);

	gpuParticle_ = std::make_unique<GPUParticle>();
	auto plane = ddm->GetDrawData(mm->GetNodeModelData(1).drawDataIndex);
	gpuParticle_->Initialize(engine_->GetComputeCommandContext(), plane);
}

void Subject::Update(Camera* camera) {
	Matrix4x4 vpMat = camera->GetVPMatrix();
	float deltatime = engine_->GetDeltaTime();
	Matrix4x4 wvp = Matrix::MakeScaleMatrix({512, 512, 512}) * vpMat;
	wvp_->CopyBuffer(&wvp, sizeof(wvp));

	animation_->Update(deltatime, vpMat);
	hitEffect_->Update(deltatime, vpMat);
	cylinder_->Update(deltatime, vpMat);
	gpuParticle_->Update(engine_->GetComputeCommandContext(), vpMat, camera->GetBillboardMatrix(), deltatime);
}

void Subject::Draw(DCC* cmdObj) {
	animation_->Draw(cmdObj);

	cube_->Draw(cmdObj);

	hitEffect_->Draw(cmdObj);

	cylinder_->Draw(cmdObj);

	gpuParticle_->Draw(cmdObj);
}
