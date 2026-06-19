#include "HitEffect.h"
#include <numbers>
#include <Utility/MatrixFactory.h>

namespace {

	void CreateRingMesh(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, float outerRadius, float innerRadius, uint32_t division) {
		vertices.clear();
		indices.clear();

		if (division < 3) {
			return;
		}

		if (innerRadius >= outerRadius) {
			return;
		}

		//=========================
		//頂点数
		//
		//1分割につき
		//外側1頂点
		//内側1頂点
		//=========================

		vertices.reserve(division * 2);

		//=========================
		//Index数
		//
		//1分割 = 2Triangle
		//2 * 3 = 6 index
		//=========================

		indices.reserve(division * 6);

		//=========================
		//Vertex生成
		//=========================

		for (uint32_t i = 0; i < division; ++i) {

			float t = static_cast<float>(i) / division;
			float angle = t * std::numbers::pi_v<float> * 2.0f;

			float c = std::cos(angle);
			float s = std::sin(angle);

			Vector3 normal = { 0.0f, 1.0f, 0.0f };

			//外周
			vertices.push_back({
				{ c * outerRadius, 0.0f, s * outerRadius, 1.0f },
				{ t, 0.0f },
				normal
			});

			//内周
			vertices.push_back({
				{ c * innerRadius, 0.0f, s * innerRadius, 1.0f },
				{ t, 1.0f },
				normal
			});
		}

		//Index生成

		for (uint32_t i = 0; i < division; ++i) {

			uint32_t next = (i + 1) % division;

			//頂点配置:
			//
			//outer = index * 2
			//inner = index * 2 + 1
			//

			uint32_t outer0 = i * 2;
			uint32_t inner0 = i * 2 + 1;

			uint32_t outer1 = next * 2;
			uint32_t inner1 = next * 2 + 1;

			//=========================
			//Triangle 1
			//
			//outer0
			//outer1
			//inner0
			//=========================

			indices.push_back(outer0);
			indices.push_back(outer1);
			indices.push_back(inner0);

			//=========================
			//Triangle 2
			//
			//inner0
			//outer1
			//inner1
			//=========================

			indices.push_back(inner0);
			indices.push_back(outer1);
			indices.push_back(inner1);
		}
	}
}

void HitEffect::Initialize(SHEngine::Engine* engine) {
	container_ = std::make_unique<SHEngine::BufferContainer>();

	auto tm = engine->GetTextureManager();
	auto mm = engine->GetModelManager();
	auto ddm = engine->GetDrawDataManager();

	{
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		CreateRingMesh(vertices, indices, 1.0f, 0.5f, 32);

		ddm->AddVertexBuffer(vertices);
		ddm->AddIndexBuffer(indices);
		int index = ddm->CreateDrawData();
		auto drawData = ddm->GetDrawData(index);

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
	float z = std::fmod(timer_ * 20, std::numbers::pi_v<float> *2);
	d_vsData_.wvp = Matrix::MakeScaleMatrix({ 6.f, 6.f, 6.f }) * Matrix::MakeRotationMatrix({ std::numbers::pi_v<float> / 2.f, 0.f, z }) * translate_ * vpMat;
	d_vsData_.uvMatrix = Matrix3x3::Identity();

	d_vsBuffer_->CopyBuffer(&d_vsData_, sizeof(d_vsData_));
	d_colorBuffer_->CopyBuffer(&color, sizeof(color));
}

void HitEffect::Draw(DCC* cmdObj) {
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
