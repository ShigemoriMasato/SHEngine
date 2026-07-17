#pragma once
#include <Render/Buffer/BufferContainer.h>

namespace SHEngine {

	class DrawData {
	public:

		void Initialize();

		void AddVertexBuffer(const GPUBuffer* buffer);
		void AddIndexBuffer(const GPUBuffer* buffer);

		std::vector<D3D12_VERTEX_BUFFER_VIEW> GetVertexBufferView() const;
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	private:

		std::vector<const GPUBuffer*> vertexBuffer_{};
		const GPUBuffer* indexBuffer_{};

	};

}
