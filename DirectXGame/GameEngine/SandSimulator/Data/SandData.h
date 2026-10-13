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

	//
	// 想定粒子サイズ = 0.1以下,0.05以上
	// cellsize = 0.4
	// cell内にぴったり入る粒子数 = 0.4 / 0.05 ^ 3 = 512
	// 圧迫を考慮して、16 ^ 3 = 4096粒子を1cellに入れることにする
	// 1cellのサイズ = 4096 * 16 + 32 = 65,568
	// grid全体の想定範囲 = 64 * 64 * 3
	// 総サイズ = 65,568 * 64 * 64 * 3 = 805,306,368 = 768MB
	//

	struct Grid {
		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t gridNum, uint32_t cellsize);

		// SRV/UAV int32_t * gridWidth * gridHeight * gridDepth * particlePerCell そのcellの中に入っている粒子のインデックス情報を格納
		SHEngine::GPUBuffer* grid;
		// SRV/UAV int32_t * gridWidth * gridHeight * gridDepth そのcellの中に入っている粒子の数を格納
		SHEngine::GPUBuffer* gridIndex;

	private:

		constexpr static uint32_t particlePerCell_ = 4096;
		constexpr static uint32_t gridWidth_ = 64;
		constexpr static uint32_t gridHeight_ = 3;
		constexpr static uint32_t gridDepth_ = 64;



	};

}
