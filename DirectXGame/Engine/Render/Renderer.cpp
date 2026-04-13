#include "Renderer.h"

SHEngine::Renderer::Renderer(DrawData& drawData) : drawData_(drawData) {
}

void SHEngine::Renderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	assert(shaderType != ShaderType::COMPUTE_SHADER);
	gpuBuffers_[shaderType][bufferType].push_back(gpuBuffer);
}

void SHEngine::Renderer::Draw(CmdObj* cmdObj) {
	auto cmdList = cmdObj->GetCommandList();
	uint32_t cmdIndex = cmdObj->GetListIndex();

	PSO::Config config;

	//PSOConfigを調整
	config.rootConfig.cbvNums = {
		int(gpuBuffers_[ShaderType::VERTEX_SHADER][BufferType::CBV].size()),
		int(gpuBuffers_[ShaderType::PIXEL_SHADER][BufferType::CBV].size())
	};
	config.rootConfig.srvNums = {
		int(gpuBuffers_[ShaderType::VERTEX_SHADER][BufferType::SRV].size()),
		int(gpuBuffers_[ShaderType::PIXEL_SHADER][BufferType::SRV].size())
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
	for (const auto& cbv : gpuBuffers_[ShaderType::VERTEX_SHADER][BufferType::CBV]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV, cmdIndex).ptr);
	}
	for (const auto& cbv : gpuBuffers_[ShaderType::PIXEL_SHADER][BufferType::CBV]) {
		cbv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		cbv->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootConstantBufferView(rootIndex++, cbv->GetGPUDescriptorHandle(BufferType::CBV, cmdIndex).ptr);
	}
	for (const auto& srv : gpuBuffers_[ShaderType::VERTEX_SHADER][BufferType::SRV]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		srv->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV, cmdIndex));
	}
	for (const auto& srv : gpuBuffers_[ShaderType::PIXEL_SHADER][BufferType::SRV]) {
		srv->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		srv->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, srv->GetGPUDescriptorHandle(BufferType::SRV, cmdIndex));
	}
	for (const auto& uav : gpuBuffers_[ShaderType::VERTEX_SHADER][BufferType::UAV]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV, cmdIndex));
	}
	for (const auto& uav : gpuBuffers_[ShaderType::PIXEL_SHADER][BufferType::UAV]) {
		uav->TransitionBarrier(D3D12_RESOURCE_STATE_GENERIC_READ);
		uav->Flush(cmdObj, cmdIndex);
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, uav->GetGPUDescriptorHandle(BufferType::UAV, cmdIndex));
	}

	if (isUseTexture_) {
		cmdList->SetGraphicsRootDescriptorTable(rootIndex++, textureStartHandle_);
	}

	cmdList->DrawIndexedInstanced(drawData_.indexNum, instanceNum_, 0, 0, 0);
}
