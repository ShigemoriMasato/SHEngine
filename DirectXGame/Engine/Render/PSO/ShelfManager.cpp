#include "ShelfManager.h"

SHEngine::PSO::ShelfManager::ShelfManager(DXDevice* device) {
	shaderShelf_ = std::make_unique<ShaderShelf>(device);
	depthStencilShelf_ = std::make_unique<DepthStencilShelf>();
	blendStateShelf_ = std::make_unique<BlendStateShelf>();
	rasterizerShelf_ = std::make_unique<RasterizerShelf>();
	rootSignatureShelf_ = std::make_unique<RootSignatureShelf>(device->GetDevice());
	inputLayoutShelf_ = std::make_unique<InputLayoutShelf>();

	d12topologyMap_[Topology::Triangle] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d12topologyMap_[Topology::Line] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	d12topologyMap_[Topology::Point] = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	d3topologyMap_[Topology::Triangle] = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	d3topologyMap_[Topology::Line] = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	d3topologyMap_[Topology::Point] = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
}

D3D12_SHADER_BYTECODE SHEngine::PSO::ShelfManager::GetShaderBytecode(ShaderType shaderType, std::string shaderName) {
	return shaderShelf_->GetShaderBytecode(shaderType, shaderName);
}

D3D12_BLEND_DESC SHEngine::PSO::ShelfManager::GetBlendState(const BlendStateID* id) {
	return blendStateShelf_->GetBlendState(id);
}

D3D12_DEPTH_STENCIL_DESC SHEngine::PSO::ShelfManager::GetDepthStencilDesc(DepthStencilID id) {
	return depthStencilShelf_->GetDepthStencilDesc(id);
}

D3D12_RASTERIZER_DESC SHEngine::PSO::ShelfManager::GetRasterizerDesc(RasterizerID id) {
	return rasterizerShelf_->GetRasterizerDesc(id);
}

ID3D12RootSignature* SHEngine::PSO::ShelfManager::GetRootSignature(const RootSignatureConfig& config) {
	return rootSignatureShelf_->GetRootSignature(config);
}

std::vector<D3D12_INPUT_ELEMENT_DESC> SHEngine::PSO::ShelfManager::GetInfluenceDesc() const {
	return inputLayoutShelf_->GetInfluenceDesc();
}

D3D12_INPUT_ELEMENT_DESC SHEngine::PSO::ShelfManager::GetInputLayoutDesc(uint32_t vertexType) const {
	return inputLayoutShelf_->GetInputLayoutDesc(vertexType);
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE SHEngine::PSO::ShelfManager::GetD3D12Topology(Topology id) const {
	return d12topologyMap_.at(id);
}

D3D_PRIMITIVE_TOPOLOGY SHEngine::PSO::ShelfManager::GetD3Topology(Topology id) const {
	return d3topologyMap_.at(id);
}

std::map<SHEngine::PSO::SamplerID, D3D12_STATIC_SAMPLER_DESC> SHEngine::PSO::ShelfManager::GetSamplers() const { 
	return rootSignatureShelf_->GetSamplers(); 
}
