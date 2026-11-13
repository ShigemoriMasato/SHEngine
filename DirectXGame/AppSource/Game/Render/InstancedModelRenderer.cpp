#include "InstancedModelRenderer.h"
#include <Camera/Camera.h>
#include <algorithm>

void InstancedModelRenderer::Initialize(const ModelData* model, uint32_t maxInstanceCount) {
	maxInstanceCount_ = std::max(1u, maxInstanceCount);
	instanceCount_ = 0;

	instanceBuffer_ = container_.Create(BufferType::SRV, sizeof(ModelInstance), maxInstanceCount_);

	renderers_.clear();
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
		auto& renderer = renderers_.emplace_back(std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, mesh));
		renderer->SetVS("Game/Actor/ActorModel.VS.hlsl");
		renderer->SetPS("Game/Actor/ActorModel.PS.hlsl");
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

	for (auto& renderer : renderers_) {
		renderer->ResetGPUBuffers();
		renderer->SetGPUBuffer(instanceBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderer->SetGPUBuffer(camera->GetVPBuffer(), ShaderType::VERTEX_SHADER, BufferType::CBV);
		renderer->Draw(dcc);
	}
}
