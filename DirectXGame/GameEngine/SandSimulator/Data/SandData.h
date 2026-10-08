#pragma once
#include <Render/Buffer/BufferContainer.h>

namespace Sand {

	struct Particle {

		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount);
		void SetProcessNum(uint32_t processNum) { countNum = processNum; count->CopyBuffer(&processNum, sizeof(int)); }

		// SRV/UAV float32_t3 粒子の数だけ
		SHEngine::GPUBuffer* position;
		// SRV/UAV float32_t3 粒子の数だけ
		SHEngine::GPUBuffer* velocity;
		// SRV/UAV float32_t 粒子の数だけ
		SHEngine::GPUBuffer* force;

		// CBV float32_t4
		SHEngine::GPUBuffer* color;
		// CBV float32_t
		SHEngine::GPUBuffer* scale;
		// CBV int32_t
		SHEngine::GPUBuffer* count;

		int countNum = 0;
	};

}
