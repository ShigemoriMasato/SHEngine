#include "SkinningProcessor.h"

void SkinningProcessor::Initialize(const ModelData* modelData, ModelDrawer* renderer) {
	modelData_ = modelData;
	renderer_ = renderer;
	easyNodes_ = renderer->GetEasyNode();

	originalPositions_.clear();
	originalNormals_.clear();
	positions_.clear();
	normals_.clear();
	influences_.clear();
	vertexNum_.clear();

	originalPositions_.reserve(modelData_->meshes.size());
	originalNormals_.reserve(modelData_->meshes.size());
	positions_.reserve(modelData_->meshes.size());
	normals_.reserve(modelData_->meshes.size());
	influences_.reserve(modelData_->meshes.size());
	vertexNum_.reserve(modelData_->meshes.size());

	for (const auto& mesh : modelData_->meshes) {
		int vertexCount = static_cast<int>(mesh.position.size());
		positions_.push_back(container_.Create(BufferType::UAV, sizeof(Vector3), vertexCount, BufferNum::Single));
		normals_.push_back(container_.Create(BufferType::UAV, sizeof(Vector3), vertexCount, BufferNum::Single));

		originalPositions_.push_back(container_.Create(BufferType::SRV, sizeof(Vector3), vertexCount, BufferNum::Single));
		originalNormals_.push_back(container_.Create(BufferType::SRV, sizeof(Vector3), vertexCount, BufferNum::Single));
		influences_.push_back(container_.Create(BufferType::SRV, sizeof(VertexInfluence), vertexCount, BufferNum::Single));
		vertexNum_.push_back(container_.Create(BufferType::CBV, sizeof(int32_t), 1, BufferNum::Single));

		originalPositions_.back()->CopyBuffer(mesh.position.data(), sizeof(Vector3) * vertexCount);
		originalNormals_.back()->CopyBuffer(mesh.normal.data(), sizeof(Vector3) * vertexCount);
		influences_.back()->CopyBuffer(mesh.vertexInfluences.data(), sizeof(VertexInfluence) * vertexCount);
		vertexNum_.back()->CopyBuffer(&vertexCount, sizeof(int32_t));
	}

	skeletonMatrices_ = container_.Create(BufferType::SRV, sizeof(WellForGPU), static_cast<uint32_t>(modelData_->skeleton.joints.size()));

	process_.Initialize();
	process_.SetShader("Skinning.CS.hlsl");

	skeletonMatricesData_.resize(modelData_->skeleton.joints.size());

	renderer->SetVertexBuffer(positions_, normals_);
}

void SkinningProcessor::Update(SHEngine::ICommandContext* cmdCtx) {
	uint32_t meshNum = static_cast<uint32_t>(modelData_->meshes.size());
	uint32_t nodeNum = static_cast<uint32_t>(modelData_->nodes.size());
	uint32_t jointNum = static_cast<uint32_t>(modelData_->skeleton.joints.size());

	auto easyNode = renderer_->GetEasyNode();

	for (uint32_t i = 0; i < nodeNum; ++i) {
		const auto& node = modelData_->nodes[i];
		if (node.skinIndex == -1) continue;

		const auto& easyNode = easyNodes_[i];
		auto& result = skeletonMatricesData_[node.skinIndex];
		Matrix4x4 parentMatrix = Matrix4x4::Identity();

		if (node.parent == -1) {
			parentMatrix = modelData_->skeleton.rootMatrix;
		} else {
			auto& parentNode = modelData_->nodes[node.parent];

			if (parentNode.skinIndex != -1) {
				parentMatrix = skeletonMatricesData_[parentNode.skinIndex].skeletonSpaceMatrix;
			} else {
				parentMatrix = modelData_->skeleton.rootMatrix;
			}
		}

		result.skeletonSpaceMatrix = easyNode.transform.Matrix() * parentMatrix;
	}

	for (uint32_t i = 0; i < jointNum; ++i) {
		const auto& joint = modelData_->skeleton.joints[i];
		auto& result = skeletonMatricesData_[i];

		result.skeletonSpaceMatrix = joint.inverseBindMatrix * result.skeletonSpaceMatrix;
		result.skeletonSpaceInverseTransposeMatrix = result.skeletonSpaceMatrix.Inverse().Transpose();
	}

	skeletonMatrices_->CopyBuffer(skeletonMatricesData_.data(), sizeof(WellForGPU) * jointNum);

	for (uint32_t i = 0; i < meshNum; ++i) {
		process_.Initialize();
		process_.SetGPUBuffers(BufferType::UAV, { positions_[i], normals_[i] });
		process_.SetGPUBuffers(BufferType::SRV, { influences_[i], skeletonMatrices_, originalPositions_[i], originalNormals_[i] });
		process_.SetGPUBuffers(BufferType::CBV, { vertexNum_[i] });
		process_.SetExecuteNum(static_cast<int>(modelData_->meshes[i].position.size()) / 128 + 1);
		process_.Execute(cmdCtx);
	}
}
