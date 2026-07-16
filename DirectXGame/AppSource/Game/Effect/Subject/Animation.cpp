#include "Animation.h"
#include <Utility/MatrixFactory.h>

void Animation_Sub::Initialize(SHEngine::Engine* engine) {
	auto mm = engine->GetModelManager();
	auto ddm = engine->GetDrawDataManager();

	const std::string filePath = "SneekWalk";

	modelData_ = mm->GetSkinningModelData(mm->LoadModel(filePath));
	auto drawData = ddm->GetDrawData(modelData_.drawDataIndex);

	renderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	renderer_->SetVS("Skinning.VS.hlsl");
	renderer_->SetPS("White.PS.hlsl");
	renderer_->SetInputLayout(SHEngine::PSO::InputLayoutID::Skinning);

	container_ = std::make_unique<SHEngine::BufferContainer>();
	wvpBuffer_ = container_->Create(BufferType::CBV, sizeof(VSData));
	boneBuffer_ = container_->Create(BufferType::SRV, sizeof(WellForGPU), 128);

	renderer_->SetGPUBuffer(wvpBuffer_, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(boneBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);

	animation_ = mm->LoadAnimation(filePath);
}

void Animation_Sub::Update(float deltaTime, const Matrix4x4& vpMatrix) {
	timer_ = std::fmod(timer_ + deltaTime, animation_.duration);
	AnimationUpdate(animation_, timer_, modelData_.skeleton);
	SkeletonUpdate(modelData_.skeleton);
	SkinningUpdate(wellForGPU_, modelData_.skinClusterData, modelData_.skeleton);

	vsData.world = Matrix::MakeScaleMatrix({4.0f, 4.0f, 4.0f}) * Matrix::MakeTranslationMatrix({ -16.0f, 0.0f, 0.0f });
	vsData.vp = vpMatrix;

	wvpBuffer_->CopyBuffer(&vsData, sizeof(vsData));
	boneBuffer_->CopyBuffer(wellForGPU_.data(), sizeof(WellForGPU) * wellForGPU_.size());
}

void Animation_Sub::Draw(DCC* cmdObj) {
	renderer_->Draw(cmdObj);
}
