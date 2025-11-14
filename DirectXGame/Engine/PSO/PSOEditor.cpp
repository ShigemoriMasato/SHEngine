#include "PSOEditor.h"

PSOEditor::PSOEditor(ID3D12Device* device) {
	logger_ = Logger::getLogger("Core");

	psoManager_ = std::make_unique<PSOManager>(device);
}

void PSOEditor::Initialize(ID3D12Device* device) {
	psoManager_->Initialize();
}

void PSOEditor::SetPixcelShader(const std::string& name) {
	nextConfig_.ps = name;
}

void PSOEditor::SetVertexShader(const std::string& name) {
	nextConfig_.vs = name;
}

void PSOEditor::SetBlendState(const BlendStateID id) {
	nextConfig_.blendID = id;
}

void PSOEditor::SetRasterizerState(const RasterizerID id) {
	nextConfig_.rasterizerID = id;
}

void PSOEditor::SetDepthStencilState(const DepthStencilID id) {
	nextConfig_.depthStencilID = id;
}

void PSOEditor::SetInputLayout(const InputLayoutID id) {
	nextConfig_.inputLayoutID = id;
}

void PSOEditor::SetOffScreen(const bool offScreen) {
	nextConfig_.isSwapChain = offScreen;
}

void PSOEditor::SetRootSignature(const RootSignatureID id) {
	nextConfig_.rootID = id;
}

void PSOEditor::SetPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY topology) {
	nextConfig_.topology = topology;
}

void PSOEditor::Setting(ID3D12GraphicsCommandList* commandList) {
	//以前の設定と同じPSOなら何もしない
	if (nowConfig_ == nextConfig_) {
		nextConfig_ = {};
		return;
	}

	if (nextConfig_.rootID != nowConfig_.rootID) {
		commandList->SetGraphicsRootSignature(psoManager_->GetRootSignature(nextConfig_.rootID));
	}
	if (nextConfig_.topology != nowConfig_.topology) {
		commandList->IASetPrimitiveTopology(nextConfig_.topology);
	}

	commandList->SetPipelineState(psoManager_->GetPSO(nextConfig_));

	nowConfig_ = nextConfig_;
	nextConfig_ = {};
}

void PSOEditor::FrameInitialize(ID3D12GraphicsCommandList* commandList) {
	nowConfig_ = {};
	commandList->SetGraphicsRootSignature(psoManager_->GetRootSignature(nowConfig_.rootID));
	commandList->SetPipelineState(psoManager_->GetPSO(nowConfig_));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
