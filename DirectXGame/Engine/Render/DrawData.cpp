#include "DrawData.h"

using namespace SHEngine;

void SHEngine::DrawData::Initialize() {
	vertexBuffer_.clear();
	indexBuffer_ = nullptr;
}

void SHEngine::DrawData::AddVertexBuffer(const GPUBuffer* buffer) {
	vertexBuffer_.push_back(buffer);
}

void SHEngine::DrawData::AddIndexBuffer(const GPUBuffer* buffer) {
	indexBuffer_ = buffer;
}

std::vector<D3D12_VERTEX_BUFFER_VIEW> SHEngine::DrawData::GetVertexBufferView() const {
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs{};
	vbvs.reserve(vertexBuffer_.size());
	for (const auto& buffer : vertexBuffer_) {
		auto& vbv = vbvs.emplace_back();
		vbv.BufferLocation = buffer->GetGPUDescriptorHandle(BufferType::CBV).ptr;
		vbv.SizeInBytes = static_cast<UINT>(buffer->GetSizeInBytes());
		vbv.StrideInBytes = static_cast<UINT>(buffer->GetStrideInBytes());
	}
	return vbvs;
}

D3D12_INDEX_BUFFER_VIEW SHEngine::DrawData::GetIndexBufferView() const {
	D3D12_INDEX_BUFFER_VIEW ibv{};
	if (indexBuffer_) {
		ibv.BufferLocation = indexBuffer_->GetGPUDescriptorHandle(BufferType::CBV).ptr;
		ibv.SizeInBytes = static_cast<UINT>(indexBuffer_->GetSizeInBytes());
		ibv.Format = DXGI_FORMAT_R32_UINT;	//uint32_tしか認めない。
	}
	return ibv;
}
