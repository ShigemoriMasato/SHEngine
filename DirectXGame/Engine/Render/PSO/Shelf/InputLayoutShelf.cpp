#include "InputLayoutShelf.h"

using namespace SHEngine::PSO;

InputLayoutShelf::InputLayoutShelf() {
	inputElementDescs_.clear();

	auto& positionDesc = inputElementDescs_[int(VertexType::Position)];
	positionDesc.SemanticName = "POSITION";
	positionDesc.SemanticIndex = 0;
	positionDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

	auto& texcoordDesc = inputElementDescs_[int(VertexType::Texcoord)];
	texcoordDesc.SemanticName = "TEXCOORD";
	texcoordDesc.SemanticIndex = 0;
	texcoordDesc.Format = DXGI_FORMAT_R32G32_FLOAT;

	auto& normalDesc = inputElementDescs_[int(VertexType::Normal)];
	normalDesc.SemanticName = "NORMAL";
	normalDesc.SemanticIndex = 0;
	normalDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

	auto& colorDesc = inputElementDescs_[int(VertexType::Color)];
	colorDesc.SemanticName = "COLOR";
	colorDesc.SemanticIndex = 0;
	colorDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	auto& weightDesc = influenceDesc_.emplace_back();
	weightDesc.SemanticName = "WEIGHT";
	weightDesc.SemanticIndex = 0;
	weightDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	auto& indexDesc = influenceDesc_.emplace_back();
	indexDesc.SemanticName = "INDEX";
	indexDesc.SemanticIndex = 0;
	indexDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
}

InputLayoutShelf::~InputLayoutShelf() {
}

D3D12_INPUT_ELEMENT_DESC InputLayoutShelf::GetInputLayoutDesc(uint32_t vertexType) const {
	return inputElementDescs_.at(vertexType);
}
