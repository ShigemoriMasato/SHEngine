#include "Renderer.h"
#include <Render/Screen/IDisplay.h>

SHEngine::Renderer::Renderer(DrawData& drawData) : drawData_(drawData) {
}

void SHEngine::Renderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	assert(shaderType != ShaderType::COMPUTE_SHADER);
	gpuBuffer->GetBufferType();
	assert((gpuBuffer->GetBufferType() & bufferType) != 0);	//gpuBufferがbufferTypeのどれかには当てはまっているか
	gpuBuffers_[bufferType][shaderType].push_back(gpuBuffer);
}

void SHEngine::Renderer::Draw(CmdObj* cmdObj) {
	auto cmdList = cmdObj->GetCommandList();
	auto display = cmdObj->GetRenderTarget();

	PSO::Config config;

	config.isSwapChain = display->GetRTVFormat() == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	//PSOConfigを調整
	config.rootConfig.cbvNums = {
		int(gpuBuffers_[BufferType::CBV][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER].size())
	};
	config.rootConfig.srvNums = {
		int(gpuBuffers_[BufferType::SRV][ShaderType::VERTEX_SHADER].size()),
		int(gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER].size())
	};
	//UAVはいつかやる
	config.rootConfig.useTexture = isUseTexture_;

	config.vs = vs_;
	config.ps = ps_;
	config.inputLayoutID = inputLayoutID_;
	config.blendID = blendID_;
	config.depthStencilID = depthStencilID_;
	config.rasterizerID = rasterizerID_;
	config.topology = topology_;
	config.isSwapChain = isSwapChain_;

	psoEditor_->SetPSO(cmdList, config);

	cmdList->IASetVertexBuffers(0, UINT(drawData_.vbv.size()), drawData_.vbv.data());
	cmdList->IASetIndexBuffer(&drawData_.ibv);

	int rootIndex = 0;
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::VERTEX_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& cbv : gpuBuffers_[BufferType::CBV][ShaderType::PIXEL_SHADER]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV).ptr);
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::VERTEX_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		srv->Flush(cmdObj);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV));
	}
	for (const auto& srv : gpuBuffers_[BufferType::SRV][ShaderType::PIXEL_SHADER]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
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

	if (isUseTexture_) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DrawIndexedInstanced(drawData_.indexNum, instanceNum_, 0, 0, 0);
}
