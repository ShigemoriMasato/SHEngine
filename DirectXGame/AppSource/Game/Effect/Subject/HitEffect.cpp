#include "HitEffect.h"
#include <Utility/MatrixFactory.h>

void HitEffect::Initialize(SHEngine::Engine* engine) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto tm = engine->GetTextureManager();
	auto mm = engine->GetModelManager();
	auto ddm = engine->GetDrawDataManager();

	auto drawData = ddm->GetDrawData(mm->GetNodeModelData(1).drawDataIndex);
	int textureIndex = tm->LoadTexture("circle2.png");

	wvp_ = container_->Create(BufferType::SRV, sizeof(Matrix4x4), spawnNum_);
	colorBuffer_ = container_->Create(BufferType::SRV, sizeof(Vector4), spawnNum_);
	auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int));
	textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(textureIndex));

	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("Simples.VS.hlsl");
	renderer_->SetPS("TexColors.PS.hlsl");
	renderer_->SetBlendState(SHEngine::PSO::BlendStateID::Normal);
	renderer_->SetDepthStencil(SHEngine::PSO::DepthStencilID::Transparent);

	renderer_->SetGPUBuffer(wvp_, ShaderType::VERTEX_SHADER, BufferType::SRV);
	renderer_->SetGPUBuffer(textureIndexBuffer, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(colorBuffer_, ShaderType::PIXEL_SHADER, BufferType::SRV);
	renderer_->SetUseTexture(true);
	renderer_->instanceNum_ = spawnNum_;

	translate_ = Matrix::MakeTranslationMatrix({ -12, 5, 0 });
	worlds_.resize(spawnNum_);
}

void HitEffect::Update(float deltaTime, const Matrix4x4& vpMat) {
	timer_ += deltaTime;
	if (timer_ >= spawnInterval_) {
		timer_ = 0.0f;
		Spawn(vpMat);
	}

	std::vector<Matrix4x4> wvpMats;
	for (const auto& world : worlds_) {
		wvpMats.push_back(world * vpMat);
	}
	wvp_->CopyBuffer(wvpMats.data(), sizeof(Matrix4x4) * spawnNum_);

	float alpha = 1.0f - (timer_ / life_);
	std::vector<Vector4> colors(spawnNum_, { 0,0,0,alpha });
	colorBuffer_->CopyBuffer(colors.data(), sizeof(Vector4) * spawnNum_);
}

void HitEffect::Draw(CmdObj* cmdObj) {
	renderer_->Draw(cmdObj);
}

void HitEffect::Spawn(const Matrix4x4& vpMat) {
	for (uint32_t i = 0; i < spawnNum_; i++) {
		Vector3 scale = { 0.4f,12.f,12.f };
		Vector3 rot = { randDist_(randEngine_), randDist_(randEngine_), randDist_(randEngine_) };
		worlds_[i] = Matrix::MakeScaleMatrix(scale) * Matrix::MakeRotationMatrix(rot) * translate_;
	}
}
