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

		std::vector<D3D12_VERTEX_BUFFER_VIEW> GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();

		uint32_t GetVertexCount() const { return vertexCount_; }
		uint32_t GetIndexCount() const { return indexBuffer_ ? indexBuffer_->GetNum() : 0; }

	private:

		std::vector<GPUBuffer*> vertexBuffer_{};
		GPUBuffer* indexBuffer_{};

		uint32_t vertexType_ = 0;

		uint32_t vertexCount_ = 0;
	};

}
