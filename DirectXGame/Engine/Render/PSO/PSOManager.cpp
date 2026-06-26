#include "PSOManager.h"
#include <cassert>

using namespace SHEngine::PSO;
Manager::Manager(DXDevice* device, ShelfManager* shelfManager) {
	binaryManager_ = std::make_unique<BinaryManager>();

	logger_ = GetLogger("Engine");

	device_ = device->GetDevice();
	shelfManager_ = shelfManager;
}

Manager::~Manager() {
	for(const auto& [config, pso] : psoMap_) {
		pso->Release();
	}
}

void Manager::Initialize() {
	//PSOをすべて廃棄
	for (auto& [config, pso] : psoMap_) {
		if (pso) {
			pso->Release();
		}
	}
	psoMap_.clear();
}

void Manager::CreatePSO(PSO::Config config) {
	//defaultとして設定したPSOを持ってくる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	//あんまり変わらないやつ
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//Configにあるやつ
	psoDesc.pRootSignature = shelfManager_->GetRootSignature(config.rootConfig);
	psoDesc.VS = shelfManager_->GetShaderBytecode(ShaderType::VERTEX_SHADER, config.vs);
	psoDesc.PS = shelfManager_->GetShaderBytecode(ShaderType::PIXEL_SHADER, config.ps);
	psoDesc.DepthStencilState = shelfManager_->GetDepthStencilDesc(config.depthStencilID);
	psoDesc.BlendState = shelfManager_->GetBlendState(config.blendID);
	psoDesc.RasterizerState = shelfManager_->GetRasterizerDesc(config.rasterizerID);
	psoDesc.InputLayout = shelfManager_->GetInputLayoutDesc(config.inputLayoutID);
	psoDesc.NumRenderTargets = config.rtvNum;
	for (uint32_t i = 0; i < config.rtvNum; ++i) {
		psoDesc.RTVFormats[i] = config.rtvFormat;
	}
	psoDesc.PrimitiveTopologyType = shelfManager_->GetD3D12Topology(config.topology);

	ID3D12PipelineState* pso = nullptr;

	HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

	if (FAILED(hr)) {
		assert(false && "Failed to create PSO");
	}

	psoMap_[config] = pso;
}

ID3D12PipelineState* Manager::GetPSO(const PSO::Config& config) {
	auto it = psoMap_.find(config);
	if (it != psoMap_.end()) {
		return it->second;
	} else {
		CreatePSO(config);
		return GetPSO(config);
	}
}

ID3D12RootSignature* Manager::GetRootSignature(const RootSignatureConfig& config) const {
	return shelfManager_->GetRootSignature(config);
}
