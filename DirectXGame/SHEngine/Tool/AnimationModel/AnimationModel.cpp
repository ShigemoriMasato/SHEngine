#include "AnimationModel.h"
#include <Utility/Easing.h>

namespace {
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
		assert(!keyframes.empty());
		if (keyframes.size() == 1 || time <= keyframes[0].time) {
			return keyframes[0].value;
		}

		for (size_t index = 0; index < keyframes.size() - 1; ++index) {
			size_t nextIndex = index + 1;

			if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
				float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
				return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
			}
		}

		return keyframes.back().value;
	}

	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
		assert(!keyframes.empty());
		if (keyframes.size() == 1 || time <= keyframes[0].time) {
			return keyframes[0].value;
		}

		for (size_t index = 0; index < keyframes.size() - 1; ++index) {
			size_t nextIndex = index + 1;

			if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
				float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
				return lerp(keyframes[index].value, keyframes[nextIndex].value, t);
			}
		}

		return keyframes.back().value;
	}
}

void AnimationModel::StaticInitialize(SHEngine::ModelManager* modelManager) {
	modelManager_ = modelManager;
}

void AnimationModel::Initialize(std::string filePath, bool canDraw, bool createDrawData) {
	container_ = std::make_unique<SHEngine::BufferContainer>();
	modelData_ = modelManager_->LoadModel(filePath);

	hasBone_ = !modelData_->skeleton.joints.empty();

	calcedJoints_.resize(modelData_->nodes.size());
	wellForGPU_.resize(modelData_->nodes.size());

	if (!createDrawData) {
		return;
	}

	for (const auto& mesh : modelData_->meshes) {
		//VBVの作成
		uint32_t  vertexCount = uint32_t(mesh.position.size());
		auto position = container_->Create(BufferType::CBV_SRV, sizeof(Vector4), vertexCount);
		auto texcoord = container_->Create(BufferType::CBV_SRV, sizeof(Vector2), vertexCount);
		auto normal = container_->Create(BufferType::CBV_SRV, sizeof(Vector3), vertexCount);

		position->CopyBuffer(mesh.position.data(), sizeof(Vector4) * vertexCount);
		texcoord->CopyBuffer(mesh.texcoord.data(), sizeof(Vector2) * vertexCount);
		normal->CopyBuffer(mesh.normal.data(), sizeof(Vector3) * vertexCount);

		vertexBuffers_.push_back({ position, texcoord, normal });

		if (hasBone_) {
			auto influences = container_->Create(BufferType::CBV_SRV, sizeof(VertexInfluence), vertexCount);
			influences->CopyBuffer(mesh.vertexInfluences.data(), sizeof(VertexInfluence) * vertexCount);
			vertexBuffers_.back().push_back(influences);
		}

		//IBVの作成
		uint32_t indexCount = uint32_t(mesh.indices.size());
		auto indexBuffer = container_->Create(BufferType::CBV_SRV, sizeof(uint32_t), indexCount);
		indexBuffer->CopyBuffer(mesh.indices.data(), sizeof(uint32_t) * indexCount);
		indexBuffers_.push_back(indexBuffer);
	}

	if (!canDraw) {
		return;
	}
}

bool AnimationModel::SetAnimation(std::string filePath, std::string animationName) {
	animation_ = modelManager_->LoadAnimation(filePath, animationName);
	return true;
}

bool AnimationModel::SetAnimation(Animation& animation) {
	animation_ = animation;
	return true;
}

void AnimationModel::Update(float& animationTimer) {
	animationTimer = std::fmod(animationTimer, animation_.duration);

	for (uint32_t i = 0; i < modelData_->nodes.size(); ++i) {
		const auto& node = modelData_->nodes[i];
		Matrix4x4 localMatrix = Matrix4x4::Identity();
		
		const auto& it = animation_.nodeAnimations.find(node.name);
		if (it == animation_.nodeAnimations.end()) {
			localMatrix = node.localMatrix;
		} else {
			localMatrix = AnimationUpdate(animation_.nodeAnimations[node.name], animationTimer);
		}

		if (node.parent == -1) {
			calcedJoints_[i].localMatrix = localMatrix;
		} else {
			calcedJoints_[i].localMatrix = localMatrix * calcedJoints_[node.parent].localMatrix;
		}

		if (hasBone_) {
			const auto& joint = modelData_->skeleton.joints[i];
			wellForGPU_[i].skeletonSpaceMatrix = joint.inverseBindMatrix * calcedJoints_[i].localMatrix;
			wellForGPU_[i].skeletonSpaceInverseTransposedMatrix = wellForGPU_[i].skeletonSpaceMatrix.Inverse().Transpose();
		}
	}
}

Matrix4x4 AnimationModel::AnimationUpdate(const NodeAnimation& animation, float time) {
	Matrix4x4 localMatrix = Matrix4x4::Identity();
	Vector3 scale, position;
	Quaternion rotation;

	scale = CalculateValue(animation.scale.keyframes, time);
	rotation = CalculateValue(animation.rotate.keyframes, time);
	position = CalculateValue(animation.position.keyframes, time);

	localMatrix = Matrix::MakeScaleMatrix(scale) * rotation.ToMatrix() * Matrix::MakeTranslationMatrix(position);
	
	return localMatrix;
}
