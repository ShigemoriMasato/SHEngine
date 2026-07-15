#include "MeshRenderer.h"
#include <Render/Screen/IDisplay.h>

void SHEngine::MeshRenderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	if (shaderType != ShaderType::MESH_SHADER && shaderType != ShaderType::PIXEL_SHADER && shaderType != ShaderType::AMPLIFICATION_SHADER) {
		logger_->info("MeshRenderer::SetGPUBuffer: This Shader is not supported.");
		return;
	}
	if (uint8_t(bufferType) & (uint8_t(bufferType) - 1)) {
		logger_->error("MeshRenderer::SetGPUBuffer: Invalid BufferType. Must be CBV, SRV, or UAV.");
		return;
	}
	if (bufferType == BufferType::ReadBack) {
		logger_->error("MeshRenderer::SetGPUBuffer: ReadBack BufferType is not supported.");
		return;
	}

	auto& rootParamConfig = psoConfig_.rootConfig.rootParams.emplace_back();
	rootParamConfig.shader = shaderType;
	rootParamConfig.registerNumber = registerCount_[shaderType][bufferType]++;
	rootParamConfig.bufferType = bufferType;

	auto& bufferConfig = bufferConfigs_.emplace_back();
	bufferConfig.buffer = gpuBuffer;
	bufferConfig.shader = shaderType;
	bufferConfig.type = bufferType;

	if (shaderType == ShaderType::PIXEL_SHADER && gpuBuffer->GetBufferType() & BufferType::UAV) {
		logger_->info("MeshRenderer::SetGPUBuffer: UAV is set in PixelShader. It will be transitioned to Common after Draw.");
		uavBuffers_.push_back(gpuBuffer);
	}
}

void SHEngine::MeshRenderer::SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType) {
	for (const auto& gpuBuffer : gpuBuffers) {
		SetGPUBuffer(gpuBuffer, shaderType, bufferType);
	}
}

void SHEngine::MeshRenderer::ResetGPUBuffers() {
}

void SHEngine::MeshRenderer::EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer) {
	int i = 0;
	for (int i = 0; i < bufferConfigs_.size(); ++i) {
		if (bufferConfigs_[i].buffer == gpuBuffer && bufferConfigs_[i].shader == shaderType && bufferConfigs_[i].type == bufferType) {
			bufferConfigs_.erase(bufferConfigs_.begin() + i);
			psoConfig_.rootConfig.rootParams.erase(psoConfig_.rootConfig.rootParams.begin() + i);
			break;
		}
	}

	if (i == bufferConfigs_.size()) {
		logger_->warn("MeshRenderer::EraseGPUBuffer: GPUBuffer not found.");
	}
}

void SHEngine::MeshRenderer::Draw(DirectCommandContext* dcc) {
	auto cmdList = dcc->GetCommandList();
	auto display = dcc->GetRenderTarget();

	if (psoConfig_.ms.empty() || psoConfig_.ps.empty()) {
		logger_->error("MeshRenderer::Draw: MS or PS is not set.");
		return;
	}
	if (groupX_ <= 0 || groupY_ <= 0 || groupZ_ <= 0) {
		return;
	}

	psoConfig_.rtvNum = display->GetRenderTargetNum();
	for (uint32_t i = 0; i < psoConfig_.rtvNum; ++i) {
		psoConfig_.rtvFormats[i] = display->GetRTVFormat();
	}

	psoEditor_->SetPSO(psoConfig_, cmdList);

	int rootIndex = 0;
	for (auto& config : bufferConfigs_) {
		config.buffer->TransitionBarrier(config.shader, config.type);
		config.buffer->Flush(cmdList);
		if (config.type == BufferType::CBV) {
			cmdList->SetGraphicsRootConstantBufferView(rootIndex++, config.buffer->GetGPUDescriptorHandle(config.type).ptr);
		} else  {
			cmdList->SetGraphicsRootDescriptorTable(rootIndex++, config.buffer->GetGPUDescriptorHandle(config.type));
		}
	}

	if (psoConfig_.rootConfig.useTexture) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DispatchMesh(groupX_, groupY_, groupZ_);

	//UAVが含まれるPSResourceはCommonに直しておく
	for (auto& uavBuffer : uavBuffers_) {
		uavBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
		uavBuffer->Flush(cmdList);
	}
}
