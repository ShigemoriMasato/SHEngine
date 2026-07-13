#include "MeshRenderer.h"
#include <Render/Screen/IDisplay.h>

void SHEngine::MeshRenderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	if (shaderType == ShaderType::COMPUTE_SHADER || shaderType == ShaderType::VERTEX_SHADER) {
		logger_->info("MeshRenderer::SetGPUBuffer: ComputeShader or VertexShader is not supported. Use Renderer instead.");
		return;
	}

	auto& rootParamConfig = psoConfig_.rootConfig.rootParams.emplace_back();
	rootParamConfig.shader = shaderType;
	rootParamConfig.registerNumber = registerCount_[shaderType][bufferType]++;
	rootParamConfig.bufferType = bufferType;
}

void SHEngine::MeshRenderer::SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType) {
	for (const auto& gpuBuffer : gpuBuffers) {
		SetGPUBuffer(gpuBuffer, shaderType, bufferType);
	}
}

void SHEngine::MeshRenderer::ResetGPUBuffers() {
}

void SHEngine::MeshRenderer::EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer) {
	auto& buffers = gpuBuffers_[bufferType][shaderType];
	buffers.erase(std::remove(buffers.begin(), buffers.end(), gpuBuffer), buffers.end());
}

void SHEngine::MeshRenderer::Draw(DirectCommandContext* dcc) {
	auto cmdList = dcc->GetCommandList();
	auto display = dcc->GetRenderTarget();

	if (psoConfig_.ms.empty() || psoConfig_.ps.empty()) {
		logger_->error("MeshRenderer::Draw: MS or PS is not set.");
		return;
	}
	if (groupX_ <= 0 || groupY_ <= 0 || groupZ_ <= 0) {
		logger_->error("MeshRenderer::Draw: Dispatch group is not set.");
		return;
	}

	psoConfig_.rtvNum = display->GetRenderTargetNum();
	for (uint32_t i = 0; i < psoConfig_.rtvNum; ++i) {
		psoConfig_.rtvFormats[i] = display->GetRTVFormat();
	}

	psoEditor_->SetPSO(psoConfig_, cmdList);

	int rootIndex = 0;
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdList);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::MESH_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		cbv->Flush(cmdList);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdList);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::MESH_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdList);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::PIXEL_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		uav->Flush(cmdList);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::MESH_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		uav->Flush(cmdList);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}

	if (psoConfig_.rootConfig.useTexture) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DispatchMesh(groupX_, groupY_, groupZ_);

	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (const auto& srvs : gpuBuffers_[BufferType::SRV]) {
		for (const auto& srv : srvs.second) {
			if (srv->GetBufferType() & uint8_t(BufferType::UAV)) {
				srv->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
				srv->Flush(cmdList);
			}
		}
	}
}
