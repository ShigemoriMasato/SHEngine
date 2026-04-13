#include "PSOEditor.h"

using namespace SHEngine::PSO;

Editor::Editor() {
	logger_ = getLogger("Engine");
}

void Editor::Initialize(DXDevice* device) {
	psoManager_ = std::make_unique<Manager>(device);
	psoManager_->Initialize();
}

void Editor::SetPSO(ID3D12GraphicsCommandList* commandList, const PSO::Config& config) {
	commandList->SetGraphicsRootSignature(psoManager_->GetRootSignature(config.rootConfig));
	commandList->IASetPrimitiveTopology(config.topology);
	commandList->SetPipelineState(psoManager_->GetPSO(config));

	nowConfig_ = config;
}

void Editor::FrameInitialize(ID3D12GraphicsCommandList* commandList) {
	nowConfig_ = {};
	commandList->SetGraphicsRootSignature(psoManager_->GetRootSignature(nowConfig_.rootConfig));
	commandList->SetPipelineState(psoManager_->GetPSO(nowConfig_));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
