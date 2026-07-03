#include "Renderer.h"
#include <Render/Screen/IDisplay.h>

SHEngine::Renderer::Renderer(const DrawData& drawData) : drawData_(drawData) {
}

void SHEngine::Renderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	assert(shaderType != ShaderType::COMPUTE_SHADER);
	gpuBuffer->GetBufferType();
	assert((gpuBuffer->GetBufferType() & bufferType) != 0);	//gpuBufferがbufferTypeのどれかには当てはまっているか
	gpuBuffers_[bufferType][shaderType].push_back(gpuBuffer);
}

void SHEngine::Renderer::SetGPUBuffers(const std::vector<GPUBuffer*>& gpuBuffers, ShaderType shaderType, BufferType bufferType) {
	for (const auto& gpuBuffer : gpuBuffers) {
		SetGPUBuffer(gpuBuffer, shaderType, bufferType);
	}
}

void SHEngine::Renderer::ResetGPUBuffers() {
	for (auto& [bufferType, bufferSplitShader] : gpuBuffers_) {
		for (auto& [shaderType, buffers] : bufferSplitShader) {
			buffers.clear();
		}
	}
}

void SHEngine::Renderer::EraseGPUBuffer(BufferType bufferType, ShaderType shaderType, GPUBuffer* gpuBuffer) {
	auto& buffers = gpuBuffers_[bufferType][shaderType];
	buffers.erase(std::remove(buffers.begin(), buffers.end(), gpuBuffer), buffers.end());
}

void SHEngine::Renderer::Draw(DirectCommandContext* dcc) {
	if (instanceNum_ <= 0) {
		return;
	}

	auto cmdObj = dcc->GetCurrentCmdObj();
	auto cmdList = cmdObj->GetCommandList();
	auto display = dcc->GetRenderTarget();

	//PSOConfigを調整
	psoConfig_.rootConfig.cbvNums = {
		int(gpuBuffers_[BufferType::CBV][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER].size())
	};
	psoConfig_.rootConfig.srvNums = {
		int(gpuBuffers_[BufferType::SRV][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER].size())
	};
	psoConfig_.rootConfig.uavNums = {
		int(gpuBuffers_[BufferType::UAV][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::UAV][ShaderType::PIXEL_SHADER].size())
	};
	psoConfig_.rootConfig.textureNums = {
		int(gpuBuffers_[BufferType::Texture2D][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::Texture2D][ShaderType::PIXEL_SHADER].size())
	};
	psoConfig_.rootConfig.ddsNums = {
		int(gpuBuffers_[BufferType::DDSTexture][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::DDSTexture][ShaderType::PIXEL_SHADER].size())
	};
	
	psoConfig_.rtvFormat = display->GetRTVFormat();
	psoConfig_.rtvNum = display->GetRenderTargetNum();

	if (!display->GetDepthTexture()) {
		psoConfig_.depthStencilID = PSO::DepthStencilID::None;
	}

	psoEditor_->SetPSO(cmdList, psoConfig_);

	cmdList->IASetVertexBuffers(0, UINT(drawData_.vbv.size()), drawData_.vbv.data());
	cmdList->IASetIndexBuffer(&drawData_.ibv);

	int rootIndex = 0;
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::VERTEX_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::VERTEX_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		srv->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::VERTEX_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	for (const auto& uav : gpuBuffers_[BufferType::UAV][ShaderType::PIXEL_SHADER]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV));
	}
	for (const auto& texture2D : gpuBuffers_[BufferType::Texture2D][ShaderType::VERTEX_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, texture2D->GetGPUDescriptorHandle(BufferType::Texture2D));
	}
	for (const auto& texture2D : gpuBuffers_[BufferType::Texture2D][ShaderType::PIXEL_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, texture2D->GetGPUDescriptorHandle(BufferType::Texture2D));
	}
	for (const auto& DDStexture : gpuBuffers_[BufferType::DDSTexture][ShaderType::VERTEX_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, DDStexture->GetGPUDescriptorHandle(BufferType::DDSTexture));
	}
	for (const auto& DDStexture : gpuBuffers_[BufferType::DDSTexture][ShaderType::PIXEL_SHADER]) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, DDStexture->GetGPUDescriptorHandle(BufferType::DDSTexture));
	}

	if (psoConfig_.rootConfig.useTexture) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DrawIndexedInstanced(drawData_.indexNum, instanceNum_, 0, 0, 0);

	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (const auto& srvs : gpuBuffers_[BufferType::SRV]) {
		for (const auto& srv : srvs.second) {
			if (srv->GetBufferType() & uint8_t(BufferType::UAV)) {
				srv->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
				srv->Flush(cmdObj);
			}
		}
	}
}
