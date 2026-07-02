#include "Cylinder.h"
#include <numbers>

struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

namespace {

	MeshData CreateCylinder(uint32_t sliceCount, float radius, float height) {

		MeshData mesh;

		if (sliceCount < 3) {
			return mesh;
		}

		constexpr float PI = std::numbers::pi_v<float>;

		//--------------------------
		// 側面
		//--------------------------

		for (uint32_t i = 0; i <= sliceCount; i++) {

			float t = static_cast<float>(i) / sliceCount;
			float theta = t * PI * 2.0f;

			float x = cosf(theta) * radius;
			float z = sinf(theta) * radius;

			Vector3 normal{ cosf(theta), 0.0f, sinf(theta) };

			// Bottom
			mesh.vertices.push_back({ {x,0.0f,z,1.0f}, {t,1.0f}, normal });

			// Top
			mesh.vertices.push_back({ {x, height , z, 1.0f}, {t,0.0f}, normal });
		}

		for (uint32_t i = 0; i < sliceCount; i++) {

			uint32_t v = i * 2;

			mesh.indices.push_back(v);
			mesh.indices.push_back(v + 1);
			mesh.indices.push_back(v + 2);

			mesh.indices.push_back(v + 2);
			mesh.indices.push_back(v + 1);
			mesh.indices.push_back(v + 3);
		}

		return mesh;
	}
}

Cylinder::Cylinder(SHEngine::Engine* engine) {
	auto ddm = engine->GetDrawDataManager();
	auto tm = engine->GetTextureManager();

	MeshData cylinderData = CreateCylinder(32, 1.0f, 2.0f);
	ddm->AddVertexBuffer(cylinderData.vertices);
	ddm->AddIndexBuffer(cylinderData.indices);
	int ddIndex = ddm->CreateDrawData();
	auto drawData = ddm->GetDrawData(ddIndex);

	container_ = std::make_unique<SHEngine::BufferContainer>();
	wvpBuffer_ = container_->Create(BufferType::CBV, sizeof(Matrix4x4));
	colorBuffer_ = container_->Create(BufferType::CBV, sizeof(Vector4));
	auto textureIndexBuffer = container_->Create(BufferType::CBV, sizeof(int), 1, BufferNum::Single);
	int textureIndex = tm->LoadTexture("gradationLine.png");
	textureIndexBuffer->CopyBuffer(&textureIndex, sizeof(int));

	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("Simple.VS.hlsl");
	renderer_->SetPS("TexColor.PS.hlsl");
	renderer_->SetGPUBuffer(wvpBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffers({ colorBuffer_, textureIndexBuffer }, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetUseTexture(true);
	renderer_->SetRasterizer(SHEngine::PSO::RasterizerID::CullNone);
	renderer_->SetSampler(SHEngine::PSO::SamplerID::ClampClamp_MinMagNearest);

	transform_.rotate.z = std::numbers::pi_v<float>;
	transform_.position = { -8.0f, -4.0f, 0.0f };
}

void Cylinder::Update(float deltaTime, Matrix4x4 vp) {
	transform_.rotate.y += deltaTime * 0.5f;
	Matrix4x4 wvp = transform_.Matrix() * vp;
	wvpBuffer_->CopyBuffer(&wvp, sizeof(Matrix4x4));

	colorBuffer_->CopyBuffer(&color_, sizeof(Vector4));
}

void Cylinder::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void Cylinder::DrawImGui() {
#ifdef USE_IMGUI

	ImGui::Begin("Cylinder");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Position", &transform_.position.x, 0.1f);
	ImGui::ColorEdit4("Color", &color_.x);
	ImGui::End();

#endif
}
