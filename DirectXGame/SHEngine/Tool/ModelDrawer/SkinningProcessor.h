#pragma once
#include <Assets/Model/ModelData.h>
#include <Compute/ComputeObject.h>
#include <Tool/ModelDrawer/ModelDrawer.h>

class SkinningProcessor {
public:

	void Initialize(const ModelData* modelData, ModelDrawer* renderer);
	void Update(SHEngine::ICommandContext* cmdCtx);

private:

	const ModelData* modelData_ = nullptr;
	const ModelDrawer* renderer_ = nullptr;

	SHEngine::BufferContainer container_{};
	SHEngine::ComputeObject process_{};

	const ModelDrawer::EasyNode* easyNodes_ = nullptr;

	struct WellForGPU {
		Matrix4x4 skeletonSpaceMatrix;
		Matrix4x4 skeletonSpaceInverseTransposeMatrix;
	};

	std::vector<SHEngine::GPUBuffer*> originalPositions_;
	std::vector<SHEngine::GPUBuffer*> originalNormals_;
	std::vector<SHEngine::GPUBuffer*> positions_;
	std::vector<SHEngine::GPUBuffer*> normals_;
	std::vector<SHEngine::GPUBuffer*> influences_;
	std::vector<SHEngine::GPUBuffer*> vertexNum_;
	SHEngine::GPUBuffer* skeletonMatrices_ = nullptr;

	std::vector<WellForGPU> skeletonMatricesData_;
};
