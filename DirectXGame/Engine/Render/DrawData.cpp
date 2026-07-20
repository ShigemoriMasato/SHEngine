#include "DrawData.h"

using namespace SHEngine;

void SHEngine::DrawData::Initialize() {
	vertexBuffer_.clear();
	vertexBuffer_.resize(uint32_t(VertexType::Count));
	indexBuffer_ = nullptr;
	vertexType_ = 0;
}

void SHEngine::DrawData::AddVertexBuffer(VertexType type, GPUBuffer* buffer) {
	if (type < VertexType::Position || type >= VertexType::Count) {
		throw std::runtime_error("Invalid vertex type.");
	}

	uint32_t typeIndex = uint32_t(type);

	if (buffer->GetSizeInBytes() == 0) {
		throw std::runtime_error("Vertex buffer is empty.");
	}

	if (vertexCount_ == 0) {
		vertexCount_ = buffer->GetNum();
	} else if (vertexCount_ != buffer->GetNum()) {
		throw std::runtime_error("Vertex count mismatch.");
	}

	vertexBuffer_[uint32_t(type)] = buffer;
	vertexType_ |= typeIndex;
}

void SHEngine::DrawData::SetIndexBuffer(GPUBuffer* buffer) {
	//uint32_tしか認めない。
	if (buffer->GetStrideInBytes() != sizeof(uint32_t)) {
		throw std::runtime_error("Index buffer must be 32-bit unsigned integer.");
	}

	if (buffer->GetSizeInBytes() == 0) {
		throw std::runtime_error("Index buffer is empty.");
	}

	indexBuffer_ = buffer;
}

std::vector<D3D12_VERTEX_BUFFER_VIEW> SHEngine::DrawData::GetVertexBufferView() {
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs{};
	vbvs.reserve(vertexBuffer_.size());
	for (auto& buffer : vertexBuffer_) {
		if (buffer) {
			buffer->Flush(nullptr);
			auto& vbv = vbvs.emplace_back();
			vbv.BufferLocation = buffer->GetGPUDescriptorHandle(BufferType::CBV).ptr;
			vbv.SizeInBytes = static_cast<UINT>(buffer->GetSizeInBytes());
			vbv.StrideInBytes = static_cast<UINT>(buffer->GetStrideInBytes());
		}
	}
	return vbvs;
}

D3D12_INDEX_BUFFER_VIEW SHEngine::DrawData::GetIndexBufferView() {
	D3D12_INDEX_BUFFER_VIEW ibv{};
	if (indexBuffer_) {
		indexBuffer_->Flush(nullptr);
		ibv.BufferLocation = indexBuffer_->GetGPUDescriptorHandle(BufferType::CBV).ptr;
		ibv.SizeInBytes = static_cast<UINT>(indexBuffer_->GetSizeInBytes());
		ibv.Format = DXGI_FORMAT_R32_UINT;
	}
	return ibv;
}
