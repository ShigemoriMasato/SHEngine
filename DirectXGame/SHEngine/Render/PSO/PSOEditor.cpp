#include "PSOEditor.h"

using namespace SHEngine::PSO;

Editor::Editor() {
	logger_ = GetLogger("Engine");
}

void Editor::Initialize(DXDevice* device, ShelfManager* shelfManager) {
	psoManager_ = std::make_unique<Manager>(device, shelfManager);
	psoManager_->Initialize();

	topologyList_.resize(static_cast<size_t>(Topology::Count));
	topologyList_[static_cast<size_t>(Topology::Triangle)] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	topologyList_[static_cast<size_t>(Topology::Line)] = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	topologyList_[static_cast<size_t>(Topology::Point)] = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
}

void Editor::SetPSO(ID3D12GraphicsCommandList* commandList, const PSO::Config& config) {
	// すでに同じ設定がされている場合は何もしない(フレーム時初めての設定の場合は例外的に設定するようにする)
	if (nowConfig_ == config && !frameFirst) {
		return;
	}

	frameFirst = false;
	commandList->SetGraphicsRootSignature(psoManager_->GetRootSignature(config.rootConfig));
	commandList->IASetPrimitiveTopology(topologyList_[uint32_t(config.topology)]);
	commandList->SetPipelineState(psoManager_->GetPSO(config));

	nowConfig_ = config;
}

void Editor::FrameInitialize() {
	frameFirst = true;
}
