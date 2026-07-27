#include "ModelDrawer.h"

void ModelDrawer::Initialize(const ModelData* modelData, std::string debugName, Type type) {
	const auto& meshes = modelData->meshes;

	uint32_t nodeNum = static_cast<uint32_t>(modelData->nodes.size());
	uint32_t meshNum = static_cast<uint32_t>(meshes.size());
	uint32_t materialNum = static_cast<uint32_t>(modelData->materials.size());

	renderers_.clear();
	renderers_.reserve(meshNum);

	transformBuffer_ = container_.Create(BufferType::SRV, sizeof(Matrix4x4), kMaxInstanceNum);
	cameraBuffer_ = container_.Create(BufferType::CBV, sizeof(Matrix4x4));
	parentMatrixBuffer_ = container_.Create(BufferType::SRV, sizeof(Matrix4x4), nodeNum);

	if (type == Type::Deco) {
		idBuffer_ = container_.Create(BufferType::SRV, sizeof(uint32_t), kMaxInstanceNum);
	}

	auto materialBuffer = container_.Create(BufferType::SRV, sizeof(MaterialData), materialNum, BufferNum::Single);

	std::vector<MaterialData> materialData;
	materialData.resize(modelData->materials.size());
	for (uint32_t i = 0; i < modelData->materials.size(); ++i) {
		materialData[i].baseColor = modelData->materials[i].baseColor;
		materialData[i].metallic = modelData->materials[i].metallic;
		materialData[i].roughness = modelData->materials[i].roughness;
		materialData[i].textureIndex = modelData->materials[i].textureIndex;
		materialData[i].normalTexture = modelData->materials[i].normalTexture;
	}
	materialBuffer->CopyBuffer(materialData.data(), sizeof(MaterialData) * materialData.size());

	for (uint32_t i = 0; i < nodeNum; ++i) {
		if (modelData->nodes[i].meshIndex == -1) continue;

		const auto& mesh = modelData->meshes[modelData->nodes[i].meshIndex];
		uint32_t materialIndexNum = static_cast<uint32_t>(mesh.primitives.size());

		auto materialIndexBuffer = container_.Create(BufferType::SRV, sizeof(uint32_t), materialIndexNum, BufferNum::Single);
		auto nodeIndexBuffer = container_.Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single);

		std::vector<uint32_t> materialIndices(materialIndexNum, 0);
		for (uint32_t j = 0; j < materialIndexNum; ++j) {
			materialIndices[j] = mesh.primitives[j].materialIndex;
		}
		materialIndexBuffer->CopyBuffer(materialIndices.data(), sizeof(uint32_t) * materialIndexNum);
		nodeIndexBuffer->CopyBuffer(&i, sizeof(uint32_t));

		auto& renderer = renderers_.emplace_back(std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, mesh));
		renderer->SetVS("Engine/ModelDrawer.VS.hlsl");
		renderer->SetPS("Engine/ModelDrawer.PS.hlsl");
		renderer->SetGPUBuffers({ transformBuffer_, parentMatrixBuffer_ }, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderer->SetGPUBuffers({ cameraBuffer_, nodeIndexBuffer }, ShaderType::VERTEX_SHADER, BufferType::CBV);
		renderer->SetGPUBuffers({ materialBuffer, materialIndexBuffer }, ShaderType::PIXEL_SHADER, BufferType::SRV);
		renderer->SetUseTexture(true);

		if (type == Type::Deco) {
			renderer->SetGPUBuffer(idBuffer_, ShaderType::PIXEL_SHADER, BufferType::SRV);
			renderer->SetPS("Engine/DecoEditor.PS.hlsl");
		}
	}

	//これを可変にするかどうかはいつかの自分に任せる
	std::vector<Matrix4x4> parentMatrices(nodeNum, Matrix4x4::Identity());
	for (uint32_t i = 0; i < nodeNum; ++i) {
		const auto& node = modelData->nodes[i];
		Matrix4x4 parent = node.parent != -1 ? parentMatrices[node.parent] : Matrix4x4::Identity();
		parentMatrices[i] = parent * node.localMatrix;
	}
	parentMatrixBuffer_->CopyBuffer(parentMatrices.data(), sizeof(Matrix4x4) * parentMatrices.size());
}

void ModelDrawer::Update(const Camera* camera) {
	Matrix4x4 tmp = camera->GetVPMatrix();
	cameraBuffer_->CopyBuffer(&tmp, sizeof(Matrix4x4));
}

void ModelDrawer::Draw(DCC* dcc) {
	for (const auto& renderer : renderers_) {
		renderer->Draw(dcc);
	}
}

void ModelDrawer::SetTransform(const std::vector<Matrix4x4>& transform) {
	uint32_t transformNum = static_cast<uint32_t>(transform.size());

	if (transformNum > kMaxInstanceNum) {
		assert(false && "Transform数が最大値を超えています。");
		transformNum = kMaxInstanceNum;
	}

	transformBuffer_->CopyBuffer(transform.data(), sizeof(Matrix4x4) * transformNum);

	for (auto& renderer : renderers_) {
		renderer->instanceNum_ = transformNum;
	}
}

void ModelDrawer::SetIDs(const std::vector<uint32_t>& ids) {
	if (idBuffer_) {
		idBuffer_->CopyBuffer(ids.data(), sizeof(uint32_t) * ids.size());
	}
}
