#include "ComputeObject.h"
#include <Utility/DirectUtilFuncs.h>

using namespace SHEngine;

PSO::CSPSOManager* ComputeObject::psoManager_ = nullptr;

ComputeObject::ComputeObject(std::string debugName) {
	debugName_ = debugName;
	logger_->info("ComputeObject {} created", debugName_);
}

void ComputeObject::Initialize() {
}

void SHEngine::ComputeObject::SetGPUBuffer(GPUBuffer* buffer, BufferType bufferType, ShaderType shaderType) {
	if (shaderType != ShaderType::COMPUTE_SHADER) {
		logger_->error("Invalid shader type for ComputeObject: {}", static_cast<int>(shaderType));
		return;
	}
	gpuBuffers_[bufferType].push_back(buffer);
}

void ComputeObject::Execute(CmdObj* cmdObj) {
	//PSOのセット
	int cbvNum = int(gpuBuffers_[BufferType::CBV].size());
	int srvNum = int(gpuBuffers_[BufferType::SRV].size());
	int uavNum = int(gpuBuffers_[BufferType::UAV].size());
	psoManager_->SetPSO(cmdObj, cbvNum, srvNum, uavNum, computeShaderName_);

	auto cmdList = cmdObj->GetCommandList();
	int cmdListIndex = cmdObj->GetListIndex();
	int rootIndex = 0;
	for(const auto& cbv : gpuBuffers_[BufferType::CBV]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj, cmdListIndex);
		cmdList->SetComputeRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV, cmdListIndex).ptr);
	}
	for(const auto& srv : gpuBuffers_[BufferType::SRV]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		srv->Flush(cmdObj, cmdListIndex);
		cmdList->SetComputeRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV, cmdListIndex));
	}
	for(const auto& uav : gpuBuffers_[BufferType::UAV]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		uav->Flush(cmdObj, cmdListIndex);
		cmdList->SetComputeRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV, cmdListIndex));
	}

	cmdList->Dispatch(threadGroupSize_.x, threadGroupSize_.y, threadGroupSize_.z);
}
