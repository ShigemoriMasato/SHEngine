#include "CSPSOManager.h"

void SHEngine::PSO::CSPSOManager::Initialize(DXDevice* device, ShelfManager* shelfManager) {
	device_ = device;
	shelfManager_ = shelfManager;
	pso_.clear();
}

const SHEngine::ShaderReflectionData* SHEngine::PSO::CSPSOManager::GetReflection(const std::string& computeShaderName) const {
	return shelfManager_->GetShaderReflection(ShaderType::COMPUTE_SHADER, directory_ + computeShaderName);
}

bool SHEngine::PSO::CSPSOManager::SetPSO(ID3D12GraphicsCommandList* cmdList, const RootSignatureConfig& rootConfig, const std::string& computeShaderName) {

	Config config{ rootConfig, computeShaderName };
	ID3D12PipelineState* pipelineState = nullptr;
	// RootSignatureはグラフィックスと共通のRootSignatureShelfで管理する(COMPUTE_SHADERの可視範囲はALL)
	auto rootSignature = shelfManager_->GetRootSignature(rootConfig);
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
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};

		// シェーダーはグラフィックスと共通のShaderShelfで管理する(Assets/Shader/Compute/以下)
		desc.CS = shelfManager_->GetShaderBytecode(ShaderType::COMPUTE_SHADER, directory_ + computeShaderName);
		desc.pRootSignature = rootSignature;

		if (desc.CS.BytecodeLength == 0) {
			return false; // シェーダーバイトコードが空の場合は処理を中断
		}

		PSO& pso = pso_[config];
		HRESULT hr = device_->GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso.pipelineState));
		if (FAILED(hr)) {
			pso_.erase(config);
			assert(false && "Failed to create compute pipeline state");
			return false;
		}

		pipelineState = pso.pipelineState.Get();
	}

	//cmdListにセットする
	cmdList->SetComputeRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	return true;
}
