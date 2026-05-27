#include "HitEffect.h"
#include <Utility/MatrixFactory.h>

void HitEffect::Initialize(SHEngine::Engine* engine) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto tm = engine->GetTextureManager();
	auto mm = engine->GetModelManager();
	auto ddm = engine->GetDrawDataManager();

	{
		auto drawData = ddm->GetDrawData(mm->GetNodeModelData(1).drawDataIndex);
		int textureIndex = tm->LoadTexture("circle2.png");

		wvp_ = container_->Create(BufferType::SRV, sizeof(Matrix4x4), spawnNum_);
		colorBuffer_ = container_->Create(BufferType::SRV, sizeof(Vector4), spawnNum_);
		auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int));
		textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(textureIndex));

		renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
		renderer_->SetVS("Simples.VS.hlsl");
		renderer_->SetPS("TexColors.PS.hlsl");
		renderer_->SetBlendState(SHEngine::PSO::BlendStateID::Add);
		renderer_->SetDepthStencil(SHEngine::PSO::DepthStencilID::Default);

		renderer_->SetGPUBuffer(wvp_, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderer_->SetGPUBuffer(colorBuffer_, ShaderType::PIXEL_SHADER, BufferType::SRV);
		renderer_->SetGPUBuffer(textureIndexBuffer, ShaderType::PIXEL_SHADER, BufferType::CBV);
		renderer_->SetUseTexture(true);
		renderer_->instanceNum_ = spawnNum_;

		translate_ = Matrix::MakeTranslationMatrix({ -12, 5, 0 });
		worlds_.resize(spawnNum_);
	}
	
	{
		auto modelData = mm->GetNodeModelData(mm->LoadModel("donut"));
		auto drawData = ddm->GetDrawData(modelData.drawDataIndex);
		int textureIndex = modelData.materials[modelData.materialIndex.front()].textureIndex;

		d_renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
		d_renderer_->SetVS("Test/Donut/UV.VS.hlsl");
		d_renderer_->SetPS("TexColor.PS.hlsl");
		d_vsBuffer_ = container_->Create(BufferType::CBV, sizeof(VSData));
		d_colorBuffer_ = container_->Create(BufferType::CBV, sizeof(Vector4));
		auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int));
		textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(textureIndex));

		d_renderer_->SetGPUBuffer(d_vsBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
		d_renderer_->SetGPUBuffer(d_colorBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
		d_renderer_->SetGPUBuffer(textureIndexBuffer, ShaderType::PIXEL_SHADER, BufferType::CBV);
		d_renderer_->SetUseTexture(true);
		d_renderer_->SetDepthStencil(SHEngine::PSO::DepthStencilID::Transparent);
		d_renderer_->SetBlendState(SHEngine::PSO::BlendStateID::Add);
		d_renderer_->SetRasterizer(SHEngine::PSO::RasterizerID::CullNone);
	}
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
	Vector4 color = { 0.8f,0.8f,0.8f,alpha };
	std::vector<Vector4> colors(spawnNum_, color);
	colorBuffer_->CopyBuffer(colors.data(), sizeof(Vector4) * spawnNum_);


	//Donut
	float z = std::fmod(timer_ * 20, std::numbers::pi_v<float> * 2);
	d_vsData_.wvp = Matrix::MakeScaleMatrix({ 6.f, 6.f, 6.f }) * Matrix::MakeRotationMatrix({std::numbers::pi_v<float> / 2.f, 0.f, z}) * translate_ * vpMat;
	d_vsData_.uvMatrix = Matrix3x3::Identity();

	d_vsBuffer_->CopyBuffer(&d_vsData_, sizeof(d_vsData_));
	d_colorBuffer_->CopyBuffer(&color, sizeof(color));
}

void HitEffect::Draw(CmdObj* cmdObj) {
	renderer_->Draw(cmdObj);
	d_renderer_->Draw(cmdObj);
}

void HitEffect::Spawn(const Matrix4x4& vpMat) {
	for (uint32_t i = 0; i < spawnNum_; i++) {
		Vector3 scale = { 0.4f,12.f,12.f };
		Vector3 rot = { randDist_(randEngine_), randDist_(randEngine_), randDist_(randEngine_) };
		worlds_[i] = Matrix::MakeScaleMatrix(scale) * Matrix::MakeRotationMatrix(rot) * translate_;
	}
}
