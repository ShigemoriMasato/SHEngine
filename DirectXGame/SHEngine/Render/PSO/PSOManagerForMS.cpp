#include "PSOManagerForMS.h"

SHEngine::PSO::ManagerMSType::ManagerMSType(DXDevice* device, ShelfManager* shelfManager) {
	device_ = device;
	shelfManager_ = shelfManager;
}

void SHEngine::PSO::ManagerMSType::Initialize() {
	psoMap_.clear();
}

void SHEngine::PSO::ManagerMSType::SetPSO(const ConfigMSType& config, ID3D12GraphicsCommandList* cmdList) {
	if (psoMap_.find(config) == psoMap_.end()) {
		PipelineStateStream psoStream = {};

		auto rootSignature = shelfManager_->GetRootSignature(config.rootConfig);
		psoStream.rootSignature = CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(rootSignature);
		psoStream.as = CD3DX12_PIPELINE_STATE_STREAM_AS(shelfManager_->GetShaderBytecode(ShaderType::AMPLIFICATION_SHADER, config.as));
		psoStream.ms = CD3DX12_PIPELINE_STATE_STREAM_MS(shelfManager_->GetShaderBytecode(ShaderType::MESH_SHADER, config.ms));
		psoStream.ps = CD3DX12_PIPELINE_STATE_STREAM_PS(shelfManager_->GetShaderBytecode(ShaderType::PIXEL_SHADER, config.ps));
		psoStream.blend = CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC(CD3DX12_BLEND_DESC(shelfManager_->GetBlendState(config.blendID)));
		psoStream.depth = CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL(CD3DX12_DEPTH_STENCIL_DESC(shelfManager_->GetDepthStencilDesc(config.depthStencilID)));
		psoStream.rasterizer = CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER(CD3DX12_RASTERIZER_DESC(shelfManager_->GetRasterizerDesc(config.rasterizerID)));
		psoStream.primitiveType = CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY(shelfManager_->GetD3D12Topology(config.topology));
		psoStream.sampleMask = CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK(D3D12_DEFAULT_SAMPLE_MASK);
		psoStream.dsvFormat = CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT(config.dsvFormat);
		DXGI_SAMPLE_DESC desc;
		desc.Count = 1;
		desc.Quality = 0;
		psoStream.sampleDesc = CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC(desc);
		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		for (int i = 0; i < 8; ++i) {
			rtvFormats.RTFormats[i] = config.rtvFormats[i];
		}
		rtvFormats.NumRenderTargets = config.rtvNum;
		psoStream.rtvFormats = CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS(rtvFormats);


		D3D12_PIPELINE_STATE_STREAM_DESC psoDesc = {};
		psoDesc.SizeInBytes = sizeof(PipelineStateStream);
		psoDesc.pPipelineStateSubobjectStream = &psoStream;

		auto& pso = psoMap_[config];
		device_->GetDevice()->CreatePipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf()));

	}

	cmdList->SetGraphicsRootSignature(shelfManager_->GetRootSignature(config.rootConfig));
	cmdList->IASetPrimitiveTopology(shelfManager_->GetD3Topology(config.topology));
	cmdList->SetPipelineState(psoMap_[config].Get());
}
