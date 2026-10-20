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

	struct GridConfig {
		Vector3 leftBottomBack = {};
		const float cellSize = 0.4f;

		const int32_t width_ = int32_t(16.0f / cellSize);
		const int32_t height_ = int32_t(16.0f / cellSize);
		const int32_t depth_ = int32_t(32.0f / cellSize);
		const int cellCount = width_ * height_ * depth_;
	};

	//
	// セルごとの数を計上							...1
	// ブロックごとに合計を計上
	// 自分より若いセルたちの合計をブロックを用いて計上	...2
	// 上を用いてパーティクルをセル番号でソートして格納	...3
	// 
	// セル番号からそのセルに含まれるパーティクルのインデックスが、1と2により3に書き込まれた場所がわかる
	//

	struct Grid {
		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount);

		//フレームごとに必要な初期化を行う
		void Update();

		void SetGridConfig(Vector3 middle);

		// SRV/UAV uint32_t * particleCount そのcellの中に入っている粒子のインデックス情報を格納
		SHEngine::GPUBuffer* particleIndicesForCell;
		// SRV/UAV uint32_t * cellCount そのcellの中に入っている粒子の数を格納
		SHEngine::GPUBuffer* particleCountInCell;
		// SRV/UAV uint32_t * cellCount particleIndicesForCellにおいて、特定のセル番号の粒子が格納される先頭のインデックスを格納する
		SHEngine::GPUBuffer* beginIndexForCell;
		// SRV/UAV uint32_t * cellCount cellをいくつかの数にまとめたとき、そのセル内に含まれるパーティクルの総数
		SHEngine::GPUBuffer* blockNumber;
		// CBV GridConfig 
		SHEngine::GPUBuffer* gridConfig;

		// UAV uint32_t * cellCount ソートした状態で粒子のインデックスを格納する際の競合回避用
		SHEngine::GPUBuffer* indexWriteBuffer;
	};

}
