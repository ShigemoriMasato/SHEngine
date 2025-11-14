#include "PSOManager.h"
#include <cassert>

std::unordered_map<D3D12_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE> PSOManager::topologyMap_{};

PSOManager::PSOManager(ID3D12Device* device) {
	shaderShelf_ = std::make_unique<ShaderShelf>();
	depthStencilShelf_ = std::make_unique<DepthStencilShelf>();
	blendStateShelf_ = std::make_unique<BlendStateShelf>();
	rasterizerShelf_ = std::make_unique<RasterizerShelf>();
	rootSignatureShelf_ = std::make_unique<RootSignatureShelf>(device);
	inputLayoutShelf_ = std::make_unique<InputLayoutShelf>();
	binaryManager_ = std::make_unique<BinaryManager>();

	logger_ = Logger::getLogger("Core");

	device_ = device;

	topologyMap_[D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	topologyMap_[D3D_PRIMITIVE_TOPOLOGY_LINELIST] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	topologyMap_[D3D_PRIMITIVE_TOPOLOGY_POINTLIST] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}

PSOManager::~PSOManager() {
#ifdef SH_RELEASE
	//ここを変更する場合はverを変更し、読み込みのコードは削除せず新しく追加すること
	binaryManager_->RegistOutput(float(1.0f), "ver");
	for (const auto& [config, pso] : psoMap_) {
		binaryManager_->RegistOutput(config.ps, "");
		binaryManager_->RegistOutput(config.vs, "");
		binaryManager_->RegistOutput((int)config.rootID, "");
		binaryManager_->RegistOutput((int)config.inputLayoutID, "");
		binaryManager_->RegistOutput((int)config.blendID, "");
		binaryManager_->RegistOutput((int)config.depthStencilID, "");
		binaryManager_->RegistOutput(config.isSwapChain, "");
		binaryManager_->RegistOutput((int)config.rasterizerID, "");
		binaryManager_->RegistOutput((int)config.topology, "");

		pso->Release();
	}

	binaryManager_->Write("UsedPSOConfig.bin");

	return;
#endif

	for(const auto& [config, pso] : psoMap_) {
		pso->Release();
	}
}

void PSOManager::Initialize() {
	//PSOをすべて廃棄
	for (auto& [config, pso] : psoMap_) {
		if (pso) {
			pso->Release();
		}
	}
	psoMap_.clear();

	auto data = binaryManager_->Read("UesdPSOConfig.bin");

	for (int i = 1; i < data.size();) {
		float version = static_cast<Value<float>*>(data.front().get())->value;

		if (version == 1.0f) {
			PSOConfig config;
			config.ps = binaryManager_->Reverse<std::string>(data[i++]);
			config.vs = binaryManager_->Reverse<std::string>(data[i++]);
			config.rootID = (RootSignatureID)binaryManager_->Reverse<int>(data[i++]);
			config.inputLayoutID = (InputLayoutID)binaryManager_->Reverse<int>(data[i++]);
			config.blendID = (BlendStateID)binaryManager_->Reverse<int>(data[i++]);
			config.depthStencilID = (DepthStencilID)binaryManager_->Reverse<int>(data[i++]);
			config.isSwapChain = binaryManager_->Reverse<bool>(data[i++]);
			config.rasterizerID = (RasterizerID)binaryManager_->Reverse<int>(data[i++]);
			config.topology = (D3D12_PRIMITIVE_TOPOLOGY)binaryManager_->Reverse<int>(data[i++]);

			CreatePSO(config);
		}
	}
}

void PSOManager::CreatePSO(PSOConfig config) {
	//defaultとして設定したPSOを持ってくる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	//あんまり変わらないやつ
	psoDesc.NumRenderTargets = 1;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Configにあるやつ
	psoDesc.pRootSignature = rootSignatureShelf_->GetRootSignature(config.rootID);
	psoDesc.VS = shaderShelf_->GetShaderBytecode(ShaderType::VERTEX_SHADER, config.vs);
	psoDesc.PS = shaderShelf_->GetShaderBytecode(ShaderType::PIXEL_SHADER, config.ps);
	psoDesc.DepthStencilState = depthStencilShelf_->GetDepthStencilDesc(config.depthStencilID);
	psoDesc.BlendState = blendStateShelf_->GetBlendState(config.blendID);
	psoDesc.RasterizerState = rasterizerShelf_->GetRasterizerDesc(config.rasterizerID);
	psoDesc.InputLayout = inputLayoutShelf_->GetInputLayoutDesc(config.inputLayoutID);

	psoDesc.RTVFormats[0] = config.isSwapChain ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.PrimitiveTopologyType = topologyMap_[config.topology];

	ID3D12PipelineState* pso = nullptr;

	HRESULT hr = device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));

	if (FAILED(hr)) {
		assert(false && "Failed to create PSO");
	}

	psoMap_[config] = pso;
}

ID3D12PipelineState* PSOManager::GetPSO(const PSOConfig& config) {
	auto it = psoMap_.find(config);
	if (it != psoMap_.end()) {
		return it->second;
	} else {
		CreatePSO(config);
		return GetPSO(config);
	}
}

ID3D12RootSignature* PSOManager::GetRootSignature(const RootSignatureID id) const {
	return rootSignatureShelf_->GetRootSignature(id);
}
