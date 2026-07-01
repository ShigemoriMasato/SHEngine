#include "MeshRenderer.h"
#include <Render/Screen/IDisplay.h>

void SHEngine::MeshRenderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	if (shaderType == ShaderType::COMPUTE_SHADER || shaderType == ShaderType::VERTEX_SHADER) {
		logger_->info("MeshRenderer::SetGPUBuffer: ComputeShader or VertexShader is not supported. Use Renderer instead.");
		return;
	}

	gpuBuffers_[bufferType][shaderType].push_back(gpuBuffer);
}

void SHEngine::MeshRenderer::SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType) {
	for (const auto& gpuBuffer : gpuBuffers) {
		SetGPUBuffer(gpuBuffer, shaderType, bufferType);
	}
}

void SHEngine::MeshRenderer::ResetGPUBuffers() {
	gpuBuffers_.clear();
}

void SHEngine::MeshRenderer::EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer) {
	auto& buffers = gpuBuffers_[bufferType][shaderType];
	buffers.erase(std::remove(buffers.begin(), buffers.end(), gpuBuffer), buffers.end());
}

void SHEngine::MeshRenderer::Draw(DirectCommandContext* dcc) {
	auto cmdList = dcc->GetCurrentCmdObj()->GetCommandList();
	auto cmdObj = dcc->GetCurrentCmdObj();
	auto display = dcc->GetRenderTarget();

	PSO::ConfigMSType psoConfig;
	psoConfig.ms = ms_;
	psoConfig.ps = ps_;
	for (int i = 0; i < 8; ++i) {
		psoConfig.blendID[i] = blendID_[i];
	}
	psoConfig.depthStencilID = depthStencilID_;
	psoConfig.rasterizerID = rasterizerID_;

	psoConfig.rtvNum = display->GetRTVNum();
	for (uint32_t i = 0; i < psoConfig.rtvNum; ++i) {
		psoConfig.rtvFormats[i] = display->GetRTVFormat();
	}
	psoConfig.dsvFormat = display->GetDepthTexture()->GetFormat();

	psoConfig.rootConfig.samplers = samplerFlag_;
	psoConfig.rootConfig.useTexture = isUseTexture_;
	psoConfig.rootConfig.cbvNums = { 
		0,
		(int)gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER].size(),
		0,
		(int)gpuBuffers_[BufferType::CBV][ShaderType::MESH_SHADER].size(),
	};
	psoConfig.rootConfig.srvNums = { 
		0,
		(int)gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER].size(),
		0,
		(int)gpuBuffers_[BufferType::SRV][ShaderType::MESH_SHADER].size(),
	};
	psoConfig.rootConfig.uavNums = { 
		0,
		(int)gpuBuffers_[BufferType::UAV][ShaderType::PIXEL_SHADER].size(),
		0,
		(int)gpuBuffers_[BufferType::UAV][ShaderType::MESH_SHADER].size(),
	};

	psoEditor_->SetPSO(psoConfig, cmdList);

	int rootIndex = 0;
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::MESH_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::MESH_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::MESH_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::PIXEL_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	for (const auto& texture2D : gpuBuffers_[BufferType::Texture2D][ShaderType::MESH_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, texture2D->GetGPUDescriptorHandle(BufferType::Texture2D));
	}
	for (const auto& texture2D : gpuBuffers_[BufferType::Texture2D][ShaderType::PIXEL_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, texture2D->GetGPUDescriptorHandle(BufferType::Texture2D));
	}
	for (const auto& DDStexture : gpuBuffers_[BufferType::DDSTexture][ShaderType::MESH_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, DDStexture->GetGPUDescriptorHandle(BufferType::DDSTexture));
	}
	for (const auto& DDStexture : gpuBuffers_[BufferType::DDSTexture][ShaderType::PIXEL_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, DDStexture->GetGPUDescriptorHandle(BufferType::DDSTexture));
	}

	if (isUseTexture_) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DispatchMesh(0, 0, 0);

	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER]) {
		if (srv->GetBufferType() & uint8_t(BufferType::UAV)) {
			srv->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
			srv->Flush(cmdObj);
		}
	}
}
