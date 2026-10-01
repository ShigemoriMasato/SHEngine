#pragma once
#include <Render/Buffer/BufferContainer.h>

namespace Sand {

	struct Particle {

		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount);

		// SRV/UAV float32_t3 粒子の数だけ
		SHEngine::GPUBuffer* position;
		// SRV/UAV float32_t3 粒子の数だけ
		SHEngine::GPUBuffer* velocity;
		// SRV/UAV float32_t 粒子の数だけ
		SHEngine::GPUBuffer* force;
		// SRV/UAV int32_t 粒子の数だけ
		SHEngine::GPUBuffer* isAlive;

		// CBV float32_t4
		SHEngine::GPUBuffer* color;
		// CBV float32_t
		SHEngine::GPUBuffer* scale;
		// CBV int32_t
		SHEngine::GPUBuffer* count;

		int countNum = 0;
	};

	struct FreeList {

		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t particleCount);

		// SRV/UAV int32_t 粒子の数だけ
		SHEngine::GPUBuffer* list;
		// CBV int32_t
		SHEngine::GPUBuffer* index;
		// CBV int32_t (Particleと併用する場合はそれと同じ値が入る)
		SHEngine::GPUBuffer* count;

		int countNum = 0;
	};

	// 配列の位置でグリッドのサイズを割り出す
	struct Grid {

		void Initialize(SHEngine::BufferContainer* bufferContainer, uint32_t verticalCount, uint32_t horizontalCount, uint32_t depthCount);

		// CBV float32_t
		SHEngine::GPUBuffer* gridSize;
		// CBV int32_t3 xyz
		SHEngine::GPUBuffer* gridCount;
		// CBV float32_t3 最も左下手前の位置
		SHEngine::GPUBuffer* gridStartPos;
		// SRV/UAV int32_t グリッドの数だけ Atomic演算のためにint32_tで作る
		SHEngine::GPUBuffer* density;
		// CBV float32_t densityがいくつで1.0fになりえるか
		SHEngine::GPUBuffer* densityMax;

		int gridNum;
	};

	// 砂に対して外力を与えるための設定項目。
	struct ShaderConfig {
		// Assets/Shader/Engine/Sand/下のファイルパス
		std::string shaderName;
		std::vector<SHEngine::GPUBuffer*> cbvs;
		std::vector<SHEngine::GPUBuffer*> srvs;
		std::vector<SHEngine::GPUBuffer*> uavs;
	};

	// 砂を追加するための設定項目。
	struct AddSandConfig {
		uint32_t particleCount;
		Vector3 position;
		float radius;
	};

}
