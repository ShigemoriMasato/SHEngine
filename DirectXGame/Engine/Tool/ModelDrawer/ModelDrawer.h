#pragma once
#include <Render/Renderer.h>
#include <Assets/Model/ModelData.h>

class ModelDrawer {
public:

	enum class Type {
		Normal,
		Deco,
	};

	struct MaterialData {
		Vector4 baseColor = { 1,1,1,1 };
		float metallic = 1.0f;
		float roughness = 1.0f;
		int textureIndex = -1;
		int normalTexture = -1;
	};

	ModelDrawer() = default;

	void Initialize(const ModelData* modelData, std::string debugName, Type type = Type::Normal);
	void Update(const Camera* camera, float deltaTime = 0.0f);
	void Draw(DCC* dcc);

	void SetMaterial(const std::vector<MaterialData>& materials);

	void SetVertexBuffer(std::vector<SHEngine::GPUBuffer*> position, std::vector<SHEngine::GPUBuffer*> normal);

	void SetTransform(const std::vector<Matrix4x4>& transform);
	void SetAnimation(const Animation& animation);
	//Deco専用関数
	void SetIDs(const std::vector<uint32_t>& ids);

	//Light実装予定

private:

	const uint32_t kMaxInstanceNum = 256;

	Transform transform_;

	SHEngine::BufferContainer container_;
	std::vector<std::unique_ptr<SHEngine::Renderer>> renderers_;

	SHEngine::GPUBuffer* idBuffer_ = nullptr;
	SHEngine::GPUBuffer* transformBuffer_ = nullptr;
	SHEngine::GPUBuffer* cameraBuffer_ = nullptr;
	SHEngine::GPUBuffer* parentMatrixBuffer_ = nullptr;
	SHEngine::GPUBuffer* materialBuffer_ = nullptr;

	std::vector<Matrix4x4> transformMatrices_;

	struct EasyNode {
		std::string name;
		QuaternionTransform transform;
	};
	std::vector<EasyNode> localTransforms_;
	Animation animation_;
	float animationTimer_ = 0.0f;
};
