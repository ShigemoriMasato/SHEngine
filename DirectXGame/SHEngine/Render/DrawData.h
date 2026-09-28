#pragma once
#include <Render/Buffer/BufferContainer.h>
#include <Assets/Model/ModelData.h>
#include <Render/PSO/Shelf/InputLayoutShelf.h>

namespace SHEngine {

	class DrawData {
	public:

		void Initialize();

		void AddVertexBuffer(VertexType type, GPUBuffer* buffer);
		void SetIndexBuffer(GPUBuffer* buffer);

		std::vector<D3D12_VERTEX_BUFFER_VIEW> GetVertexBufferView(ID3D12GraphicsCommandList* cmdList);
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(ID3D12GraphicsCommandList* cmdList);

		uint32_t GetVertexCount() const { return vertexCount_; }
		uint32_t GetIndexCount() const { return indexBuffer_ ? indexBuffer_->GetNum() : 0; }

		// @brief 指定された頂点タイプのGPUBufferを取得する。未登録の場合はnullptrを返す。
		GPUBuffer* GetVertexBuffer(VertexType type) const;

	private:

		std::vector<GPUBuffer*> vertexBuffer_{};
		GPUBuffer* indexBuffer_{};

		uint32_t vertexType_ = 0;

		uint32_t vertexCount_ = 0;
	};

}
