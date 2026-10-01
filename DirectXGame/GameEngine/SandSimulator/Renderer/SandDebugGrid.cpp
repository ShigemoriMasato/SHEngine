#include "SandDebugGrid.h"
#include <imgui/imgui.h>

Sand::DebugGrid::DebugGrid(Grid* gridData, ModelData* cubeModel, SHEngine::BufferContainer* container) {
	gridData_ = gridData;

	matrixBuffer_ = container->Create(BufferType::CBV, sizeof(MatrixData));
	materialBuffer_ = container->Create(BufferType::CBV, sizeof(MaterialData));

	renderer_ = std::make_unique<SHEngine::Renderer>(SHEngine::VertexType::Default, cubeModel->meshes.front());
	renderer_->SetVS("Engine/SandSimulator/GridDebug.VS.hlsl");
	renderer_->SetPS("Engine/SandSimulator/GridDebug.PS.hlsl");

	renderer_->SetGPUBuffers({ matrixBuffer_, gridData_->gridCount, gridData_->gridSize }, ShaderType::VERTEX_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(materialBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
	renderer_->SetGPUBuffer(gridData_->density, ShaderType::PIXEL_SHADER, BufferType::SRV);
}

void Sand::DebugGrid::Initialize() {
}

void Sand::DebugGrid::Update(Camera* camera) {
	Matrix4x4 vpMatrix = camera->GetVPMatrix();
	matrixBuffer_->CopyBuffer(&vpMatrix, sizeof(vpMatrix));
	materialBuffer_->CopyBuffer(&materialData_, sizeof(materialData_));
}

void Sand::DebugGrid::Draw(DCC* dcc) {
	renderer_->Draw(dcc);
}

void Sand::DebugGrid::DrawImGui() {
#ifdef USE_IMGUI

	if (openOrder_) {
		openOrder_ = false;
		isMaterialWindowOpened_ = true;
		ImGui::SetNextWindowFocus();
	}

	if (ImGui::Begin("Grid Debug Material", &isMaterialWindowOpened_)) {
		ImGui::Checkbox("Visible", &isVisible_);
		ImGui::ColorEdit3("Upper Color", &materialData_.upperColor.x);
		ImGui::ColorEdit3("Lower Color", &materialData_.lowerColor.x);
		ImGui::SliderFloat("Alpha", &materialData_.alpha, 0.0f, 1.0f);
		ImGui::SliderFloat("Outline Width", &materialData_.outlineWidth, 0.0f, 1.0f);
	}
	ImGui::End();

#endif
}

void Sand::DebugGrid::Save(BinaryManager& binManager) const {
	binManager.Register(&isVisible_);
	binManager.Register(&isMaterialWindowOpened_);
	binManager.Register(&materialData_.upperColor);
	binManager.Register(&materialData_.alpha);
	binManager.Register(&materialData_.lowerColor);
	binManager.Register(&materialData_.outlineWidth);
}

void Sand::DebugGrid::Load(BinaryManager& binManager) {
	isVisible_ = binManager.Reverse<bool>();
	isMaterialWindowOpened_ = binManager.Reverse<bool>();
	materialData_.upperColor = binManager.Reverse<Vector3>();
	materialData_.alpha = binManager.Reverse<float>();
	materialData_.lowerColor = binManager.Reverse<Vector3>();
	materialData_.outlineWidth = binManager.Reverse<float>();
}
