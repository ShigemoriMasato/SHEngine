#include "MeshRenderer.h"
#include <Render/Screen/IDisplay.h>

void SHEngine::MeshRenderer::SetASBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer) {
	SetGPUBuffer(ShaderType::AMPLIFICATION_SHADER, bufferType, registerNumber, gpuBuffer);
}

void SHEngine::MeshRenderer::SetASBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers) {
	SetGPUBuffers(ShaderType::AMPLIFICATION_SHADER, bufferType, startRegister, gpuBuffers);
}

void SHEngine::MeshRenderer::EraseASBuffer(BufferType bufferType, int registerNumber) {
	EraseGPUBuffer(ShaderType::AMPLIFICATION_SHADER, bufferType, registerNumber);
}

void SHEngine::MeshRenderer::SetMSBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer) {
	SetGPUBuffer(ShaderType::MESH_SHADER, bufferType, registerNumber, gpuBuffer);
}

void SHEngine::MeshRenderer::SetMSBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers) {
	SetGPUBuffers(ShaderType::MESH_SHADER, bufferType, startRegister, gpuBuffers);
}

void SHEngine::MeshRenderer::EraseMSBuffer(BufferType bufferType, int registerNumber) {
	EraseGPUBuffer(ShaderType::MESH_SHADER, bufferType, registerNumber);
}

void SHEngine::MeshRenderer::SetPSBuffer(BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer) {
	SetGPUBuffer(ShaderType::PIXEL_SHADER, bufferType, registerNumber, gpuBuffer);
}

void SHEngine::MeshRenderer::SetPSBuffers(BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers) {
	SetGPUBuffers(ShaderType::PIXEL_SHADER, bufferType, startRegister, gpuBuffers);
}

void SHEngine::MeshRenderer::ErasePSBuffer(BufferType bufferType, int registerNumber) {
	EraseGPUBuffer(ShaderType::PIXEL_SHADER, bufferType, registerNumber);
}

void SHEngine::MeshRenderer::SetGPUBuffer(ShaderType shaderType, BufferType bufferType, int registerNumber, GPUBuffer* gpuBuffer) {
	if (bufferType != BufferType::CBV && bufferType != BufferType::SRV && bufferType != BufferType::UAV) {
		logger_->error("MeshRenderer::SetGPUBuffer: Invalid BufferType. Must be CBV, SRV, or UAV.");
		return;
	}
	if (!gpuBuffer) {
		logger_->error("MeshRenderer::SetGPUBuffer: gpuBuffer is nullptr.");
		return;
	}

	// 同じシェーダー・種類・レジスタが登録済みなら差し替える
	auto& rootParams = psoConfig_.rootConfig.rootParams;
	size_t index = rootParams.size();
	for (size_t i = 0; i < rootParams.size(); ++i) {
		if (rootParams[i].shader == shaderType && rootParams[i].bufferType == bufferType && rootParams[i].registerNumber == registerNumber) {
			index = i;
			break;
		}
	}

	if (index < rootParams.size()) {
		// 差し替え前のBufferをUAVリストから外す
		GPUBuffer* oldBuffer = bufferConfigs_[index].buffer;
		std::erase(uavBuffers_, oldBuffer);
		bufferConfigs_[index].buffer = gpuBuffer;
	} else {
		rootParams.push_back({ shaderType, registerNumber, bufferType });

		auto& bufferConfig = bufferConfigs_.emplace_back();
		bufferConfig.buffer = gpuBuffer;
		bufferConfig.shader = shaderType;
		bufferConfig.type = bufferType;
	}

	if (shaderType == ShaderType::PIXEL_SHADER && gpuBuffer->GetBufferType() & BufferType::UAV) {
		logger_->info("MeshRenderer::SetGPUBuffer: UAV is set in PixelShader. It will be transitioned to Common after Draw.");
		uavBuffers_.push_back(gpuBuffer);
	}
}

void SHEngine::MeshRenderer::SetGPUBuffers(ShaderType shaderType, BufferType bufferType, int startRegister, const std::vector<GPUBuffer*>& gpuBuffers) {
	for (int i = 0; i < int(gpuBuffers.size()); ++i) {
		SetGPUBuffer(shaderType, bufferType, startRegister + i, gpuBuffers[i]);
	}
}

void SHEngine::MeshRenderer::EraseGPUBuffer(ShaderType shaderType, BufferType bufferType, int registerNumber) {
	auto& rootParams = psoConfig_.rootConfig.rootParams;
	for (size_t i = 0; i < rootParams.size(); ++i) {
		if (rootParams[i].shader == shaderType && rootParams[i].bufferType == bufferType && rootParams[i].registerNumber == registerNumber) {
			std::erase(uavBuffers_, bufferConfigs_[i].buffer);
			bufferConfigs_.erase(bufferConfigs_.begin() + i);
			rootParams.erase(rootParams.begin() + i);
			return;
		}
	}

	logger_->warn("MeshRenderer::EraseGPUBuffer: GPUBuffer not found.");
}

void SHEngine::MeshRenderer::ResetGPUBuffers() {
	bufferConfigs_.clear();
	uavBuffers_.clear();
	psoConfig_.rootConfig.rootParams.clear();
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

	bool result = psoEditor_->SetPSO(psoConfig_, cmdList);

	if (!result) {
		return;
	}

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

	cmdList->DispatchMesh(groupX_, groupY_, groupZ_);

	//UAVが含まれるPSResourceはCommonに直しておく
	for (auto& uavBuffer : uavBuffers_) {
		uavBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
		uavBuffer->Flush(cmdList);
	}
}
