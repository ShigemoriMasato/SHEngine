#include "ModelDrawer.h"
#include <Utility/Easing.h>

namespace {
	template <typename T>
	T EaseAnimationKey(const AnimationCurve<T>& curve, float time) {
		for (uint32_t i = 0; i < uint32_t(curve.keyframes.size()); ++i) {
			if (time < curve.keyframes[i].time) {
				int prevIndex = i - 1;
				int nextIndex = i;

				if (prevIndex < 0) {
					prevIndex = int(curve.keyframes.size() - 1);
				}

				auto prevKey = curve.keyframes[prevIndex];
				auto nextKey = curve.keyframes[nextIndex];
				
				float t = (time - prevKey.time) / (nextKey.time - prevKey.time);

				T value = lerp(prevKey.value, nextKey.value, t);
				return value;
			}
		}
		assert(false && "アニメーションの時間が範囲外です。");
		return T();
	}
}

void ModelDrawer::Initialize(const ModelData* modelData, std::string debugName, Type type) {
	const auto& meshes = modelData->meshes;

	uint32_t nodeNum = static_cast<uint32_t>(modelData->nodes.size());
	uint32_t meshNum = static_cast<uint32_t>(meshes.size());
	uint32_t materialNum = static_cast<uint32_t>(modelData->materials.size());

	renderers_.clear();
	renderers_.reserve(meshNum);

	transformBuffer_ = container_.Create(BufferType::SRV, sizeof(Matrix4x4), kMaxInstanceNum);
	parentMatrixBuffer_ = container_.Create(BufferType::SRV, sizeof(Matrix4x4), nodeNum);

	if (type == Type::Deco) {
		idBuffer_ = container_.Create(BufferType::SRV, sizeof(uint32_t), kMaxInstanceNum);
	}

	materialBuffer_ = container_.Create(BufferType::SRV, sizeof(MaterialData), materialNum, BufferNum::Single);

	std::vector<MaterialData> materialData;
	materialData.resize(modelData->materials.size());
	for (uint32_t i = 0; i < modelData->materials.size(); ++i) {
		materialData[i].baseColor = modelData->materials[i].baseColor;
		materialData[i].metallic = modelData->materials[i].metallic;
		materialData[i].roughness = modelData->materials[i].roughness;
		materialData[i].textureIndex = modelData->materials[i].textureIndex;
		materialData[i].normalTexture = modelData->materials[i].normalTexture;
	}
	materialBuffer_->CopyBuffer(materialData.data(), sizeof(MaterialData) * materialData.size());

	materialIndexBuffer_.reserve(nodeNum);
	nodeIndexBuffer_.reserve(nodeNum);
	for (uint32_t i = 0; i < nodeNum; ++i) {
		if (modelData->nodes[i].meshIndex == -1) continue;

		const auto& mesh = modelData->meshes[modelData->nodes[i].meshIndex];
		uint32_t materialIndexNum = static_cast<uint32_t>(mesh.primitives.size());

		auto materialIndexBuffer = materialIndexBuffer_.emplace_back(container_.Create(BufferType::SRV, sizeof(uint32_t), materialIndexNum, BufferNum::Single));
		auto nodeIndexBuffer = nodeIndexBuffer_.emplace_back(container_.Create(BufferType::CBV, sizeof(uint32_t), 1, BufferNum::Single));

		std::vector<uint32_t> materialIndices(materialIndexNum, 0);
		for (uint32_t j = 0; j < materialIndexNum; ++j) {
			materialIndices[j] = mesh.primitives[j].materialIndex;
		}
		materialIndexBuffer->CopyBuffer(materialIndices.data(), sizeof(uint32_t) * materialIndexNum);
		nodeIndexBuffer->CopyBuffer(&i, sizeof(uint32_t));

		auto& renderer = renderers_.emplace_back(std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, mesh));
		renderer->SetVS("Engine/ModelDrawer.VS.hlsl");
		renderer->SetPS("Engine/ModelDrawer.PS.hlsl");
		renderer->SetUseTexture(true);

		if (type == Type::Deco) {
			renderer->SetPS("Engine/DecoEditor.PS.hlsl");
		}
	}

	transformMatrices_.resize(nodeNum, Matrix4x4::Identity());
	localTransforms_.resize(nodeNum);
	for (uint32_t i = 0; i < nodeNum; ++i) {
		const auto& node = modelData->nodes[i];
		localTransforms_[i].name = node.name;
		localTransforms_[i].transform.position = node.localTransform.position;
		localTransforms_[i].transform.rotate= node.localTransform.rotate;
		localTransforms_[i].transform.scale = node.localTransform.scale;
	}
}

void ModelDrawer::Update(const Camera* camera, float deltaTime) {
	Matrix4x4 tmp = camera->GetVPMatrix();

	animationTimer_ = std::fmod(animationTimer_ + deltaTime, animation_.duration);
	
	if (!animation_.nodeAnimations.empty()) {
		for (auto& easyNode : localTransforms_) {
			const auto& it = animation_.nodeAnimations.find(easyNode.name);
			if (it == animation_.nodeAnimations.end()) continue;
			const auto& nodeAnimation = it->second;

			easyNode.transform.position = EaseAnimationKey(nodeAnimation.position, animationTimer_);
			easyNode.transform.rotate = EaseAnimationKey(nodeAnimation.rotate, animationTimer_);
			easyNode.transform.scale = EaseAnimationKey(nodeAnimation.scale, animationTimer_);
		}
	}

	for (uint32_t i = 0; i < (uint32_t)localTransforms_.size(); ++i) {
		transformMatrices_[i] = localTransforms_[i].transform.Matrix();
	}

	parentMatrixBuffer_->CopyBuffer(transformMatrices_.data(), sizeof(Matrix4x4) * transformMatrices_.size());

	cameraBuffer_ = camera->GetVPBuffer();
}

void ModelDrawer::Draw(DCC* dcc) {
	if (!cameraBuffer_) {
		return;
	}

	for (uint32_t i = 0; i < (uint32_t)renderers_.size(); ++i) {
		renderers_[i]->ResetGPUBuffers();
		renderers_[i]->SetGPUBuffers({ transformBuffer_, parentMatrixBuffer_ }, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderers_[i]->SetGPUBuffers({ cameraBuffer_, nodeIndexBuffer_[i] }, ShaderType::VERTEX_SHADER, BufferType::CBV);
		renderers_[i]->SetGPUBuffers({ materialBuffer_, materialIndexBuffer_[i] }, ShaderType::PIXEL_SHADER, BufferType::SRV);
		if (idBuffer_) {
			renderers_[i]->SetPS("Engine/DecoEditor.PS.hlsl");
			renderers_[i]->SetGPUBuffer(idBuffer_, ShaderType::PIXEL_SHADER, BufferType::SRV);
		}
		renderers_[i]->SetUseTexture(true);
		renderers_[i]->Draw(dcc);
	}
}

void ModelDrawer::NormalDraw(DCC* dcc) {
	if (!cameraBuffer_) {
		return;
	}

	for (uint32_t i = 0; i < (uint32_t)renderers_.size(); ++i) {
		renderers_[i]->ResetGPUBuffers();
		renderers_[i]->SetPS("Engine/ModelDrawer.PS.hlsl");
		renderers_[i]->SetGPUBuffers({ transformBuffer_, parentMatrixBuffer_ }, ShaderType::VERTEX_SHADER, BufferType::SRV);
		renderers_[i]->SetGPUBuffers({ cameraBuffer_, nodeIndexBuffer_[i] }, ShaderType::VERTEX_SHADER, BufferType::CBV);
		renderers_[i]->SetGPUBuffers({ materialBuffer_, materialIndexBuffer_[i] }, ShaderType::PIXEL_SHADER, BufferType::SRV);
		renderers_[i]->SetUseTexture(true);
		renderers_[i]->Draw(dcc);
	}
}

void ModelDrawer::SetMaterial(const std::vector<MaterialData>& materials) {
	size_t materialSize = materials.size() * sizeof(MaterialData);
	if(materialSize > materialBuffer_->GetSizeInBytes()) {
		assert(false && "Material数が最大値を超えています。");
	}

	materialBuffer_->CopyBuffer(materials.data(), materialSize);
}

void ModelDrawer::SetVertexBuffer(std::vector<SHEngine::GPUBuffer*> position, std::vector<SHEngine::GPUBuffer*> normal) {
	for (uint32_t i = 0; i < renderers_.size(); ++i) {
		renderers_[i]->SetVertexBuffer(SHEngine::VertexType::Position, position[i]);
		renderers_[i]->SetVertexBuffer(SHEngine::VertexType::Normal, normal[i]);
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

void ModelDrawer::SetAnimation(const Animation& animation) {
	animation_ = animation;
	animationTimer_ = 0.0f;
}

void ModelDrawer::SetIDs(const std::vector<uint32_t>& ids) {
	if (idBuffer_) {
		idBuffer_->CopyBuffer(ids.data(), sizeof(uint32_t) * ids.size());
	}
}
