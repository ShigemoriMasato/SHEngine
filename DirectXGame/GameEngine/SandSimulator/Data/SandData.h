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
	// grid全体の想定範囲 = 32 * 32 * 8
	// gridの総数 = 32 / 0.4 * 32 / 0.4 * 8 / 0.4 = 80 * 80 * 20 = 128,000
	// 総サイズ = 65,568 * 128,000 = 8,392,960,000 = 8.4GB
	//

	struct GridConfig {
		const float cellSize = 0.4f;
		Vector3 leftBottomBack = { -128.f, 0.0f, -128.f };

		const int32_t width_ = int32_t(256.0f / cellSize);
		const int32_t depth_ = int32_t(64.0f / cellSize);
		const int32_t height_ = int32_t(256.0f / cellSize);
		const int cellCount = width_ * height_ * depth_;
	};

	struct Grid {
		void Initialize(SHEngine::BufferContainer* bufferContainer, float cellSize);
		void SetGridConfig(Vector3 middle, float cellSize);

		// SRV/UAV int32_t * gridWidth * gridHeight * gridDepth * particlePerCell そのcellの中に入っている粒子のインデックス情報を格納
		SHEngine::GPUBuffer* particleIndicesForCell;
		// SRV/UAV int32_t * gridWidth * gridHeight * gridDepth そのcellの中に入っている粒子の数を格納
		SHEngine::GPUBuffer* particleCountInCell;
		// CBV GridConfig 
		SHEngine::GPUBuffer* gridConfig;

	};

}
