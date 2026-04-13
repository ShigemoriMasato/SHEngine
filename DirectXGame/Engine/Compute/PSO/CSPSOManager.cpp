#include "CSPSOManager.h"

void SHEngine::PSO::CSPSOManager::Initialize(DXDevice* device) {
	device_ = device;
	rootSignature_ = std::make_unique<CSRootSignature>();
	rootSignature_->Initialize(device);
	shaderShelf_ = std::make_unique<CSShaderShelf>();
	shaderShelf_->Initialize(device);
	pso_.clear();
}

void SHEngine::PSO::CSPSOManager::SetPSO(CmdObj* cmdObj, int cbv, int srv, int uav, std::string computeShaderName) {

	Config config{ cbv, srv, uav, computeShaderName };
	ID3D12PipelineState* pipelineState = nullptr;
	auto rootSignature = rootSignature_->GetRootSignature(cbv, srv, uav);
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

		HRESULT hr = device_->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso.pipelineState));
		assert(SUCCEEDED(hr) && "Failed to create compute pipeline state");

		pipelineState = pso.pipelineState.Get();
	}

	//cmdListにセットする
	auto cmdList = cmdObj->GetCommandList();
	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);
}
