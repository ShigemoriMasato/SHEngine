#include "CSPSOManager.h"

void SHEngine::PSO::CSPSOManager::Initialize(DXDevice* device, std::map<SamplerID, D3D12_STATIC_SAMPLER_DESC> samplers) {
	device_ = device;
	rootSignature_ = std::make_unique<CSRootSignature>();
	rootSignature_->Initialize(device, samplers);
	shaderShelf_ = std::make_unique<CSShaderShelf>();
	shaderShelf_->Initialize(device);
	pso_.clear();
}

bool SHEngine::PSO::CSPSOManager::SetPSO(ID3D12GraphicsCommandList* cmdList, const RootSignatureConfig& rootConfig, const std::string& computeShaderName) {

	Config config{ rootConfig, computeShaderName };
	ID3D12PipelineState* pipelineState = nullptr;
	auto rootSignature = rootSignature_->GetRootSignature(rootConfig);
	if (!rootSignature) {
		return false;
	}
	const auto& it = pso_.find(config);

	if (it != pso_.end()) {
		//作成済みならそれを使う

		pipelineState = it->second.pipelineState.Get();
	}
	else {
		//未作成なら作成する

		PSO& pso = pso_[config];
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};

		desc.CS = shaderShelf_->GetShaderBytecode(computeShaderName);
		desc.pRootSignature = rootSignature;

		if (desc.CS.BytecodeLength == 0) {
			return false; // シェーダーバイトコードが空の場合は処理を中断
		}

		HRESULT hr = device_->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso.pipelineState));
		assert(SUCCEEDED(hr) && "Failed to create compute pipeline state");

		pipelineState = pso.pipelineState.Get();
	}

	//cmdListにセットする
	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	return true;
}
