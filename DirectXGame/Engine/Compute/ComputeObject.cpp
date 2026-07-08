#include "ComputeObject.h"
#include <Utility/DirectUtilFuncs.h>

using namespace SHEngine;

PSO::CSPSOManager* ComputeObject::psoManager_ = nullptr;
D3D12_GPU_DESCRIPTOR_HANDLE ComputeObject::textureStartHandle_ = {};

ComputeObject::ComputeObject(std::string debugName) {
	debugName_ = debugName;
	logger_->info("ComputeObject {} created", debugName_);
}

void SHEngine::ComputeObject::StaticInitialize(PSO::CSPSOManager* psoManager, D3D12_GPU_DESCRIPTOR_HANDLE textureStartHandle) {
	psoManager_ = psoManager;
	textureStartHandle_ = textureStartHandle;
}

void ComputeObject::Initialize() {
	gpuBuffers_.clear();
}

void SHEngine::ComputeObject::SetGPUBuffer(BufferType bufferType, GPUBuffer* buffer) {
	gpuBuffers_[bufferType].push_back(buffer);
}

void SHEngine::ComputeObject::SetGPUBuffers(BufferType bufferType, std::vector<GPUBuffer*> buffers) {
	for (auto buffer : buffers) {
		SetGPUBuffer(bufferType, buffer);
	}
}

void SHEngine::ComputeObject::SetUseTexture(bool useTexture) {
	useTexture_ = useTexture;
}

void ComputeObject::Execute(SHEngine::ICommandContext* commandContext) {
	//PSOのセット
	auto cmdList = commandContext->GetCommandList();
	int cbvNum = int(gpuBuffers_[BufferType::CBV].size());
	int srvNum = int(gpuBuffers_[BufferType::SRV].size());
	int uavNum = int(gpuBuffers_[BufferType::UAV].size());

	if (useTexture_ && samplerID_ == 0) {
		samplerID_ = uint32_t(PSO::SamplerID::Default);
	}

	psoManager_->SetPSO(cmdList, cbvNum, srvNum, uavNum, useTexture_, samplerID_, computeShaderName_);

	int rootIndex = 0;
	for(const auto& cbv : gpuBuffers_[BufferType::CBV]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		cbv->Flush(cmdList);
		cmdList->SetComputeRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for(const auto& srv : gpuBuffers_[BufferType::SRV]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdList);
		cmdList->SetComputeRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for(const auto& uav : gpuBuffers_[BufferType::UAV]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		uav->Flush(cmdList);
		cmdList->SetComputeRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	if (useTexture_) {
		cmdList->SetComputeRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->Dispatch(threadGroupSize_.x, threadGroupSize_.y, threadGroupSize_.z);
}
