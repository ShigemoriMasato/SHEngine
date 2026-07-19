#include "Renderer.h"
#include <Render/Screen/IDisplay.h>

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
	bufferConfig.buffers.push_back(gpuBuffer);
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

void SHEngine::Renderer::Draw(DirectCommandContext* dcc) {
	if (instanceNum_ <= 0) {
		return;
	}

	auto cmdList = dcc->GetCommandList();
	auto display = dcc->GetRenderTarget();

	for (uint32_t i = 0; i < drawData_.size(); ++i) {
		auto& drawData = drawData_[i];

		uint32_t vertexCount = drawData.GetVertexCount();
		uint32_t indexCount = drawData.GetIndexCount();

		if (vertexCount <= 0) {
			assert(false && "Renderer::Draw: VertexCount or IndexCount is zero.");
			return;
		}

		psoConfig_.rtvFormat = display->GetRTVFormat();
		psoConfig_.rtvNum = display->GetRenderTargetNum();
		psoConfig_.isDSV = display->GetDepthTexture() != nullptr;
		psoConfig_.dsvFormat = display->GetDepthTexture() ? display->GetDepthTexture()->GetFormat() : DXGI_FORMAT_UNKNOWN;

		if (!display->GetDepthTexture()) {
			psoConfig_.depthStencilID = PSO::DepthStencilID::None;
		}

		auto vbvs = drawData.GetVertexBufferView();
		auto ibv = drawData.GetIndexBufferView();

		psoEditor_->SetPSO(cmdList, psoConfig_);

		cmdList->IASetVertexBuffers(0, UINT(vbvs.size()), vbvs.data());
		if (indexCount > 0) {
			cmdList->IASetIndexBuffer(&ibv);
		}

		int rootIndex = 0;
		for (auto& config : bufferConfigs_) {
			auto& buffer = config.buffers[i % config.buffers.size()];

			buffer->TransitionBarrier(config.shader, config.type);
			buffer->Flush(cmdList);
			if (config.type == BufferType::CBV) {
				cmdList->SetGraphicsRootConstantBufferView(rootIndex++, buffer->GetGPUDescriptorHandle(config.type).ptr);
			} else {
				cmdList->SetGraphicsRootDescriptorTable(rootIndex++, buffer->GetGPUDescriptorHandle(config.type));
			}
		}

		if (psoConfig_.rootConfig.useTexture) {
			cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
		}

		if (indexCount > 0) {
			cmdList->DrawIndexedInstanced(indexCount, instanceNum_, 0, 0, 0);
		} else {
			cmdList->DrawInstanced(vertexCount, instanceNum_, 0, 0);
		}
	}

	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (auto& uavBuffer : uavBuffers_) {
		uavBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
		uavBuffer->Flush(cmdList);
	}
}
