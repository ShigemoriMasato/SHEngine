#include "Renderer.h"
#include <Render/Screen/IDisplay.h>

SHEngine::Renderer::Renderer(const DrawData& drawData) : drawData_(drawData) {
}

void SHEngine::Renderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	if (shaderType != ShaderType::VERTEX_SHADER && shaderType != ShaderType::PIXEL_SHADER) {
		assert(false && "Renderer::SetGPUBuffer Not use Resource of VS or PS");	//VERTEX_SHADERかPIXEL_SHADER以外は使えない
	}
	assert((gpuBuffer->GetBufferType() & bufferType) != 0);	//gpuBufferがbufferTypeのどれかには当てはまっているか

	auto& rootParamConfig = psoConfig_.rootConfig.rootParams.emplace_back();
	rootParamConfig.shader = shaderType;
	rootParamConfig.registerNumber = registerCount_[shaderType][bufferType]++;
	rootParamConfig.bufferType = bufferType;

	auto& bufferConfig = bufferConfigs_.emplace_back();
	bufferConfig.buffer = gpuBuffer;
	bufferConfig.shader = shaderType;
	bufferConfig.type = bufferType;
}

void SHEngine::Renderer::SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType) {
	for (const auto& gpuBuffer : gpuBuffers) {
		SetGPUBuffer(gpuBuffer, shaderType, bufferType);
	}
}

void SHEngine::Renderer::ResetGPUBuffers() {
	bufferConfigs_.clear();
	registerCount_.clear();
	uavBuffers_.clear();
	psoConfig_.rootConfig.rootParams.clear();
}

void SHEngine::Renderer::EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer) {
	int i = 0;
	for (int i = 0; i < bufferConfigs_.size(); ++i) {
		if (bufferConfigs_[i].buffer == gpuBuffer && bufferConfigs_[i].shader == shaderType && bufferConfigs_[i].type == bufferType) {
			bufferConfigs_.erase(bufferConfigs_.begin() + i);
			psoConfig_.rootConfig.rootParams.erase(psoConfig_.rootConfig.rootParams.begin() + i);
			break;
		}
	}

	if (i == bufferConfigs_.size()) {
		assert(false && "Renderer::EraseGPUBuffer: GPUBuffer not found.");
	}
}

void SHEngine::Renderer::Draw(DirectCommandContext* dcc) {
	if (instanceNum_ <= 0) {
		return;
	}

	auto cmdList = dcc->GetCommandList();
	auto display = dcc->GetRenderTarget();

	psoConfig_.rtvFormat = display->GetRTVFormat();
	psoConfig_.rtvNum = display->GetRenderTargetNum();
	psoConfig_.isDSV = display->GetDepthTexture() != nullptr;
	psoConfig_.dsvFormat = display->GetDepthTexture() ? display->GetDepthTexture()->GetFormat() : DXGI_FORMAT_UNKNOWN;

	if (!display->GetDepthTexture()) {
		psoConfig_.depthStencilID = PSO::DepthStencilID::None;
	}

	psoEditor_->SetPSO(cmdList, psoConfig_);

	cmdList->IASetVertexBuffers(0, UINT(drawData_.vbv.size()), drawData_.vbv.data());
	cmdList->IASetIndexBuffer(&drawData_.ibv);

	int rootIndex = 0;
	for (auto& config : bufferConfigs_) {
		config.buffer->TransitionBarrier(config.shader, config.type);
		config.buffer->Flush(cmdList);
		if (config.type == BufferType::CBV) {
			cmdList->SetGraphicsRootConstantBufferView(rootIndex++, config.buffer->GetGPUDescriptorHandle(config.type).ptr);
		} else {
			cmdList->SetGraphicsRootDescriptorTable(rootIndex++, config.buffer->GetGPUDescriptorHandle(config.type));
		}
	}

	if (psoConfig_.rootConfig.useTexture) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DrawIndexedInstanced(drawData_.indexNum, instanceNum_, 0, 0, 0);

	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (auto& uavBuffer : uavBuffers_) {
		uavBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
		uavBuffer->Flush(cmdList);
	}
}
