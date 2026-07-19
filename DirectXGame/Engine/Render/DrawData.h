#pragma once
#include <Render/Buffer/BufferContainer.h>
#include <Assets/Model/ModelData.h>

namespace SHEngine {

	enum class VertexType {
		Position = 1 << 0,
		Texcoord = 1 << 1,
		Normal = 1 << 2,
		Color = 1 << 3,
		Influence = 1 << 4,
		Count = 5
	};

	class DrawData {
	public:

		void Initialize();

		void AddVertexBuffer(VertexType type, const GPUBuffer* buffer);
		void AddIndexBuffer(const GPUBuffer* buffer);

		std::vector<D3D12_VERTEX_BUFFER_VIEW> GetVertexBufferView() const;
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

		uint32_t GetVertexCount() const { return vertexCount_; }
		uint32_t GetIndexCount() const { return indexBuffer_ ? indexBuffer_->GetNum() : 0; }

	private:

		static inline const std::vector<size_t> vertexStrideInBytes_ = {
			sizeof(Vector3),
			sizeof(Vector2),
			sizeof(Vector3),
			sizeof(Vector4),
			sizeof(VertexInfluence),
		};

		std::vector<const GPUBuffer*> vertexBuffer_{};
		const GPUBuffer* indexBuffer_{};

		uint32_t vertexCount_ = 0;
	};

}
