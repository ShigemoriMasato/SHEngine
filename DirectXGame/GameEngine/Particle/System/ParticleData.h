#pragma once
#include <Render/Buffer/BufferContainer.h>

namespace Particle {

	struct Data {
		void Initialize(SHEngine::BufferContainer* container, uint32_t particleCount);

		// @brief SRV/UAV float32_t3 * particleCount
		SHEngine::GPUBuffer* positions;
		// @brief SRV/UAV float32_t * particleCount
		SHEngine::GPUBuffer* scales;
		// @brief SRV/UAV float32_t4 * particleCount
		SHEngine::GPUBuffer* colors;
		// @brief SRV/UAV uint32_t * particleCount 親行列のインデックス(設定方法は後で決める)
		SHEngine::GPUBuffer* parentIndices;
		// @brief SRV float32_t4x4 * 256 親行列
		SHEngine::GPUBuffer* parents;

		// @brief CBV uint32_t
		SHEngine::GPUBuffer* count;

		uint32_t countNum = 0;
		const uint32_t parentMatricesMaxNum = 256;
	};

	struct FreeList {
		void Initialize(SHEngine::BufferContainer* container, uint32_t capacityCount);

		// @brief UAV uint32_t * particleCount
		SHEngine::GPUBuffer* freeList;
		// @brief UAV int32_t
		SHEngine::GPUBuffer* freeListCount;

		// @brief CBV uint32_t
		SHEngine::GPUBuffer* capacity;

		uint32_t capacityNum = 0;
	};

}
