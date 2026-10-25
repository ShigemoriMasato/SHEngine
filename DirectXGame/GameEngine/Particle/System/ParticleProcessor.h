#pragma once
#include "ParticleData.h"
#include <Compute/ComputeObject.h>

namespace Particle {

	class Processor {
	public:

		Processor(Data* data, FreeList* freeList);

		void Initialize(ICC* icc);
		void Update();

		void Allocate(SHEngine::GPUBuffer* indexList, SHEngine::GPUBuffer* emitterID, SHEngine::GPUBuffer* countBuffer, int countNum, ICC* icc);

		// アプリケーション層からの隠ぺいのためSystemではなくProcessorに作成。
		void SetParentMatrix(uint32_t emitterID, const Matrix4x4& matrix);

		uint32_t GetKey() const { return key_; }

	private:

		const uint32_t key_;

		Data* data_ = nullptr;
		FreeList* freeList_ = nullptr;

		std::vector<Matrix4x4> parentMatrices_;

		std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> allocate_ = nullptr;

	};

}
