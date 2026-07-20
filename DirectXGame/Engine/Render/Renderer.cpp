#include "Renderer.h"
#include <Render/Screen/IDisplay.h>

SHEngine::Renderer::Renderer(VertexType type, const Mesh& modelData) {
	uint32_t vertexTypeCount = std::popcount(static_cast<uint32_t>(type));
	uint32_t vertexCount = uint32_t(modelData.position.size());
	uint32_t indexCount = uint32_t(modelData.indices.size());

	container_ = std::make_unique<BufferContainer>(vertexTypeCount + (indexCount > 0 ? 1 : 0));
	drawData_.Initialize();

	static const std::vector<VertexType> vertexTypes = {
		VertexType::Position,
		VertexType::Texcoord,
		VertexType::Normal,
		VertexType::Color,
		VertexType::Influence
	};

	static const std::map<uint32_t, size_t> vertexStrideInBytes = {
		{ uint32_t(VertexType::Position), sizeof(Vector3) },
		{ uint32_t(VertexType::Texcoord), sizeof(Vector2) },
		{ uint32_t(VertexType::Normal), sizeof(Vector3) },
		{ uint32_t(VertexType::Color), sizeof(Vector4) },
		{ uint32_t(VertexType::Influence), sizeof(VertexInfluence) }
	};

	vertexBuffers_.resize(uint32_t(VertexType::Count));
	for (const auto& vertexType : vertexTypes) {
		if (static_cast<uint32_t>(type) & static_cast<uint32_t>(vertexType)) {
			auto buffer = container_->Create(BufferType::CBV, vertexStrideInBytes.at(static_cast<uint32_t>(vertexType)), vertexCount, BufferNum::Single);
			vertexBuffers_[static_cast<uint32_t>(vertexType)] = buffer;
			switch (vertexType) {
			case VertexType::Position:
				buffer->CopyBuffer(modelData.position.data(), vertexCount * vertexStrideInBytes.at(static_cast<uint32_t>(vertexType)));
				break;
			case VertexType::Texcoord:
				buffer->CopyBuffer(modelData.texcoord.data(), vertexCount * vertexStrideInBytes.at(static_cast<uint32_t>(vertexType)));
				break;
			case VertexType::Normal:
				buffer->CopyBuffer(modelData.normal.data(), vertexCount * vertexStrideInBytes.at(static_cast<uint32_t>(vertexType)));
				break;
			case VertexType::Color:
				buffer->CopyBuffer(modelData.color.data(), vertexCount * vertexStrideInBytes.at(static_cast<uint32_t>(vertexType)));
				break;
			case VertexType::Influence:
				//いつかやる
				break;
			}
			
			drawData_.AddVertexBuffer(vertexType, buffer);
		}
	}

	if (indexCount > 0) {
		indexBuffer_ = container_->Create(BufferType::CBV, sizeof(uint32_t), indexCount, BufferNum::Single);
		indexBuffer_->CopyBuffer(modelData.indices.data(), indexCount * sizeof(uint32_t));
		
		drawData_.SetIndexBuffer(indexBuffer_);
	}

	vertexType_ = type;
}

SHEngine::Renderer::Renderer(VertexType type) {
	vertexBuffers_.resize(uint32_t(VertexType::Count));
	drawData_.Initialize();
	vertexType_ = type;
}

void SHEngine::Renderer::SetVertexBuffer(VertexType type, GPUBuffer* gpuBuffer) {
	vertexBuffers_[uint32_t(type)] = gpuBuffer;
	drawData_.AddVertexBuffer(type, gpuBuffer);
	vertexType_ = static_cast<VertexType>(static_cast<uint32_t>(vertexType_) | static_cast<uint32_t>(type));
}

void SHEngine::Renderer::SetIndexBuffer(GPUBuffer* gpuBuffer) {
	indexBuffer_ = gpuBuffer;
	drawData_.SetIndexBuffer(gpuBuffer);
}

void SHEngine::Renderer::SetGPUBuffer(GPUBuffer* gpuBuffer, ShaderType shaderType, BufferType bufferType) {
	if (shaderType != ShaderType::VERTEX_SHADER && shaderType != ShaderType::PIXEL_SHADER) {
		logger_->warn("Renderer::SetGPUBuffer: Not use Resource of VS or PS");
		assert(false);
	}
	assert((gpuBuffer->GetBufferType() & bufferType) != 0);

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

void SHEngine::Renderer::Draw(DirectCommandContext* dcc) {
	if (instanceNum_ <= 0) {
		return;
	}

	auto cmdList = dcc->GetCommandList();
	auto display = dcc->GetRenderTarget();

	uint32_t vertexCount = drawData_.GetVertexCount();
	uint32_t indexCount = drawData_.GetIndexCount();

	if (vertexCount <= 0) {
		assert(false && "Renderer::Draw: VertexCount or IndexCount is zero.");
		return;
	}

	psoConfig_.rtvFormat = display->GetRTVFormat();
	psoConfig_.rtvNum = display->GetRenderTargetNum();
	psoConfig_.isDSV = display->GetDepthTexture() != nullptr;
	psoConfig_.dsvFormat = display->GetDepthTexture() ? display->GetDepthTexture()->GetFormat() : DXGI_FORMAT_UNKNOWN;
	psoConfig_.inputLayoutID = vertexType_;

	if (!display->GetDepthTexture()) {
		psoConfig_.depthStencilID = PSO::DepthStencilID::None;
	}

	auto vbvs = drawData_.GetVertexBufferView();
	auto ibv = drawData_.GetIndexBufferView();

	psoEditor_->SetPSO(cmdList, psoConfig_);

	cmdList->IASetVertexBuffers(0, UINT(vbvs.size()), vbvs.data());
	if (indexCount > 0) {
		cmdList->IASetIndexBuffer(&ibv);
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

	if (indexCount > 0) {
		cmdList->DrawIndexedInstanced(indexCount, instanceNum_, 0, 0, 0);
	} else {
		cmdList->DrawInstanced(vertexCount, instanceNum_, 0, 0);
	}


	//SRVのなかで、UAVが含まれるPSResourceはCommonに直しておく
	for (auto& uavBuffer : uavBuffers_) {
		uavBuffer->TransitionBarrier(D3D12_RESOURCE_STATE_COMMON);
		uavBuffer->Flush(cmdList);
	}
}