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
	ResetGPUBuffers();
	SetExecuteNum(1);
	SetUseTexture(false);
	SetSamplerID(0);
}

void SHEngine::ComputeObject::SetExecuteNum(int executeNum, int threadSize) {
	if (executeNum <= 0) {
		threadGroupSize_ = { 0, 0, 0 };
	}
	int factNum = executeNum / threadSize;
	threadGroupSize_.x = factNum % 65535 + 1;
	threadGroupSize_.y = (factNum / 65535) % 65535 + 1;
	// int32_tで表せるはずがないので1とする。
	threadGroupSize_.z = 1;
}

void SHEngine::ComputeObject::SetGPUBuffer(BufferType bufferType, int registerNumber, GPUBuffer* buffer) {
	if (bufferType != BufferType::CBV && bufferType != BufferType::SRV && bufferType != BufferType::UAV) {
		logger_->error("ComputeObject({})::SetGPUBuffer: Invalid BufferType. Must be CBV, SRV, or UAV.", debugName_);
		return;
	}
	if (!buffer) {
		logger_->error("ComputeObject({})::SetGPUBuffer: buffer is nullptr.", debugName_);
		return;
	}

	// 同じレジスタが登録済みなら差し替える
	for (size_t i = 0; i < rootConfig_.rootParams.size(); ++i) {
		const auto& param = rootConfig_.rootParams[i];
		if (param.bufferType == bufferType && param.registerNumber == registerNumber) {
			gpuBuffers_[i] = buffer;
			return;
		}
	}

	rootConfig_.rootParams.push_back({ ShaderType::COMPUTE_SHADER, registerNumber, bufferType });
	gpuBuffers_.push_back(buffer);
}

void SHEngine::ComputeObject::SetGPUBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& buffers) {
	for (int i = 0; i < int(buffers.size()); ++i) {
		SetGPUBuffer(bufferType, startRegister + i, buffers[i]);
	}
}

void SHEngine::ComputeObject::ResetGPUBuffers() {
	gpuBuffers_.clear();
	rootConfig_.rootParams.clear();
}

void SHEngine::ComputeObject::SetUseTexture(bool useTexture) {
	rootConfig_.useTexture = useTexture;
}

void ComputeObject::Execute(SHEngine::ICommandContext* commandContext) {
	if (threadGroupSize_.x == 0 || threadGroupSize_.y == 0 || threadGroupSize_.z == 0) {
		return;
	}

	//PSOのセット
	auto cmdList = commandContext->GetCommandList();
	if (rootConfig_.useTexture && rootConfig_.samplers == 0) {
		rootConfig_.samplers = uint32_t(PSO::SamplerID::Default);
	}

	bool result = psoManager_->SetPSO(cmdList, rootConfig_, computeShaderName_);
	if (!result) {
		return;
	}

	// 登録した順番がそのままRootParameterIndexになる
	UINT rootIndex = 0;
	for (size_t i = 0; i < gpuBuffers_.size(); ++i) {
		GPUBuffer* buffer = gpuBuffers_[i];
		BufferType type = rootConfig_.rootParams[i].bufferType;
		switch (type) {
		case BufferType::CBV:
			buffer->TransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			buffer->Flush(cmdList);
			cmdList->SetComputeRootConstantBufferView(rootIndex++, buffer->GetGPUDescriptorHandle(type).ptr);
			break;
		case BufferType::SRV:
			buffer->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			buffer->Flush(cmdList);
			cmdList->SetComputeRootDescriptorTable(rootIndex++, buffer->GetGPUDescriptorHandle(type));
			break;
		case BufferType::UAV:
			buffer->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			buffer->Flush(cmdList);
			cmdList->SetComputeRootDescriptorTable(rootIndex++, buffer->GetGPUDescriptorHandle(type));
			break;
		default:
			break;
		}
	}
	if (rootConfig_.useTexture) {
		cmdList->SetComputeRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->Dispatch(threadGroupSize_.x, threadGroupSize_.y, threadGroupSize_.z);
}
