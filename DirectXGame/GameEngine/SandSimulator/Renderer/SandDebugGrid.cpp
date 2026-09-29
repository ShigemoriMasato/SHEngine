#include "SandDebugGrid.h"

Sand::DebugGrid::DebugGrid(Grid* gridData, ModelData* cubeModel, SHEngine::BufferContainer* container) {
	gridData_ = gridData;

	drawDataBuffer_ = container->Create(BufferType::CBV, sizeof(DebugGridData), 1);

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, cubeModel->meshes.front());
	renderer_->SetVS("Engine/SandSimulator/GridDebug.VS.hlsl");
	renderer_->SetPS("Engine/SandSimulator/GridDebug.PS.hlsl");

}

void Sand::DebugGrid::Initialize() {
}

void Sand::DebugGrid::Update() {
}

void Sand::DebugGrid::Draw(const DCC* dcc) {
}
