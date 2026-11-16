#include "InstancedModelRenderer.h"
#include <Camera/Camera.h>
#include <algorithm>

void InstancedModelRenderer::Initialize(const ModelData* model, uint32_t maxInstanceCount) {
	maxInstanceCount_ = std::max(1u, maxInstanceCount);
	instanceCount_ = 0;

	instanceBuffer_ = container_.Create(BufferType::SRV, sizeof(ModelInstance), maxInstanceCount_);

	renderers_.clear();
	materialBuffers_.clear();
	modelRadius_ = 0.5f;
	if (!model) {
		return;
	}

	// モデル空間での半径(原点から最も遠い頂点までの距離)
	float maxLength = 0.0f;
	for (const Mesh& mesh : model->meshes) {
		for (const Vector3& position : mesh.position) {
			maxLength = std::max(maxLength, position.Length());
		}
	}
	if (maxLength > 1.0e-4f) {
		modelRadius_ = maxLength;
	}

	// モデルのメッシュごとにRendererを作る(頂点バッファはRenderer内部で1回だけ作成される)
	for (const Mesh& mesh : model->meshes) {
		if (mesh.position.empty()) {
			continue;
		}
		// assimpの仕様上 1メッシュ = 1マテリアルなので、先頭プリミティブのマテリアルを使う
		ModelMaterial material{};
		if (!mesh.primitives.empty()) {
			const uint32_t materialIndex = mesh.primitives.front().materialIndex;
			if (materialIndex < model->materials.size()) {
				material.baseColor = model->materials[materialIndex].baseColor;
				material.textureIndex = model->materials[materialIndex].textureIndex;
			}
		}
		SHEngine::GPUBuffer* materialBuffer = materialBuffers_.emplace_back(container_.Create(BufferType::CBV, sizeof(ModelMaterial), 1, BufferNum::Single));
		materialBuffer->CopyBuffer(&material, sizeof(ModelMaterial));

		auto& renderer = renderers_.emplace_back(std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, mesh));
		renderer->SetVS("Game/Actor/ActorModel.VS.hlsl");
		renderer->SetPS("Game/Actor/ActorModel.PS.hlsl");
		renderer->SetUseTexture(true);
		renderer->instanceNum_ = 0;
	}
}

void InstancedModelRenderer::SetInstances(const std::vector<ModelInstance>& instances) {
	instanceCount_ = static_cast<uint32_t>(std::min(instances.size(), static_cast<size_t>(maxInstanceCount_)));

	if (instanceCount_ > 0) {
		instanceBuffer_->CopyBuffer(instances.data(), sizeof(ModelInstance) * instanceCount_);
	}

	for (auto& renderer : renderers_) {
		renderer->instanceNum_ = instanceCount_;
	}
}

void InstancedModelRenderer::Draw(DCC* dcc, const Camera* camera) {
	if (instanceCount_ == 0 || !camera) {
		return;
	}

	for (size_t i = 0; i < renderers_.size(); ++i) {
		auto& renderer = renderers_[i];
		renderer->ResetGPUBuffers();
		renderer->SetGPUBuffer(instanceBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderer->SetGPUBuffer(camera->GetVPBuffer(), ShaderType::VERTEX_SHADER, BufferType::CBV);
		renderer->SetGPUBuffer(materialBuffers_[i], ShaderType::PIXEL_SHADER, BufferType::CBV);
		renderer->Draw(dcc);
	}
}
