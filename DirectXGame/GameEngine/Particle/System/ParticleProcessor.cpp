#include "ParticleProcessor.h"
#include <random>

namespace {
	std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<uint32_t> distribution(0, UINT32_MAX);
}

Particle::Processor::Processor(Data* data, FreeList* freeList) : key_(distribution(generator)) {
	auto createCO = [](const std::string& shaderName) {
		auto co = std::make_unique<SHEngine::ComputeObject>(shaderName);
		co->SetShader("Particle/System/" + shaderName + ".CS.hlsl");
		return co;
		};

	data_ = data;
	freeList_ = freeList;

	initialize_ = createCO("Initialize");
	initialize_->SetExecuteNum(data->countNum, 1024);
	initialize_->SetGPUBuffers(BufferType::CBV, { data->count });
	initialize_->SetGPUBuffers(BufferType::UAV, { freeList->freeList, freeList->freeListCount, data->positions, data->scales, data->colors, data->parentIndices });

	allocate_ = createCO("Allocate");
	//GPUBufferはどこに割り当てるかでBufferの種類が変わるので割愛

	parentMatrices_.resize(data->parentMatricesMaxNum, Matrix4x4::Identity());
}

void Particle::Processor::Initialize(ICC* icc) {
	initialize_->Execute(icc);
}

void Particle::Processor::Update() {
	data_->parents->CopyBuffer(parentMatrices_.data(), sizeof(Matrix4x4) * parentMatrices_.size());
}

void Particle::Processor::Allocate(SHEngine::GPUBuffer* indexList, SHEngine::GPUBuffer* emitterID, SHEngine::GPUBuffer* countBuffer, int countNum, ICC* icc) {
	allocate_->Initialize();
	allocate_->SetGPUBuffers(BufferType::CBV, { countBuffer, emitterID });
	allocate_->SetGPUBuffers(BufferType::SRV, { freeList_->freeList });
	allocate_->SetGPUBuffers(BufferType::UAV, { freeList_->freeListCount, indexList, data_->parentIndices });
	allocate_->SetExecuteNum(countNum, 256 * 32);
	allocate_->Execute(icc);
}

void Particle::Processor::SetParentMatrix(uint32_t emitterID, const Matrix4x4& matrix) {
	parentMatrices_[emitterID] = matrix;
}
