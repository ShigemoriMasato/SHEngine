#include "PSOManagerForMS.h"

SHEngine::PSO::ManagerMSType::ManagerMSType(DXDevice* device, ShelfManager* shelfManager) {
	device_ = device;
	shelfManager_ = shelfManager;
}

void SHEngine::PSO::ManagerMSType::Initialize() {
	psoMap_.clear();
}

bool SHEngine::PSO::ManagerMSType::SetPSO(const ConfigMSType& config, ID3D12GraphicsCommandList* cmdList) {
	const auto& it = psoMap_.find(config);
	if (it == psoMap_.end()) {
		PipelineStateStream psoStream = {};

		D3D12_SHADER_BYTECODE as, ms, ps;
		if (!config.as.empty()) {
			as = shelfManager_->GetShaderBytecode(ShaderType::AMPLIFICATION_SHADER, config.as);
			if (as.BytecodeLength == 0) {
				psoMap_[config] = nullptr;
				return false;
			}
		}

		ms = shelfManager_->GetShaderBytecode(ShaderType::MESH_SHADER, config.ms);
		ps = shelfManager_->GetShaderBytecode(ShaderType::PIXEL_SHADER, config.ps);

		if (ms.BytecodeLength == 0 || ps.BytecodeLength == 0) {
			psoMap_[config] = nullptr;
			return false;
		}

		auto rootSignature = shelfManager_->GetRootSignature(config.rootConfig);
		psoStream.rootSignature = CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE(rootSignature);
		psoStream.as = CD3DX12_PIPELINE_STATE_STREAM_AS(as);
		psoStream.ms = CD3DX12_PIPELINE_STATE_STREAM_MS(ms);
		psoStream.ps = CD3DX12_PIPELINE_STATE_STREAM_PS(ps);
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
		HRESULT hr = device_->GetDevice()->CreatePipelineState(&psoDesc, IID_PPV_ARGS(pso.GetAddressOf()));

		if (FAILED(hr)) {
			pso = nullptr;
			return false;
		}
	}

	auto pso = psoMap_[config].Get();
	if (!pso) {
		return false;
	}

	cmdList->SetGraphicsRootSignature(shelfManager_->GetRootSignature(config.rootConfig));
	cmdList->IASetPrimitiveTopology(shelfManager_->GetD3Topology(config.topology));
	cmdList->SetPipelineState(pso);

	return true;
}
