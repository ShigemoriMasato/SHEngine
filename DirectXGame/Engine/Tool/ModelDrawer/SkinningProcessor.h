#pragma once
#include <Assets/Model/ModelData.h>
#include <Compute/ComputeObject.h>

class SkinningProcessor {
public:

	void Initialize(const ModelData* modelData);
	void Update(SHEngine::ICommandContext* cmdCtx, std::vector<Matrix4x4> nodeMatrices);

private:

	const ModelData* modelData_ = nullptr;

	SHEngine::BufferContainer container_{};
	SHEngine::ComputeObject process_{};

	std::vector<SHEngine::GPUBuffer*> positions_;
	std::vector<SHEngine::GPUBuffer*> normals_;

	SHEngine::GPUBuffer* boneMatrices_ = nullptr;


};
