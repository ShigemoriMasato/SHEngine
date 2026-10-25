#pragma once
#include <Compute/ComputeObject.h>
#include <Particle/System/ParticleData.h>
#include <Particle/System/ParticleProcessor.h>

namespace Particle {

	class Emitter {
	public:

		Emitter(uint32_t emitterID, uint32_t particleCount, Data* data, FreeList* freeList, Processor* processor, ICC* icc, SHEngine::BufferContainer* container);

		// @brief 毎フレーム実行する処理を追加
		// @param output 追加したComputeObjectのポインタを返す。
		// @return 追加したComputeObjectのIDを返す。EraseProcessで削除する際に使用する。
		uint32_t AddProcess(SHEngine::ComputeObject*& output);

		// @brief 一度実行したら破棄される処理を追加
		// @param output 追加したComputeObjectのポインタを返す。
		// @return 追加したComputeObjectのIDを返す。EraseProcessで削除する際に使用する。
		void AddTriggerProcess(SHEngine::ComputeObject*& output);

		// @param id AddProcessで使用したID。
		void EraseProcess(uint32_t id);

		// @param 実行順 [Trigger->Process]
		void Update(uint32_t processorKey, ICC* icc);

		void SetParentMatrix(const Matrix4x4& matrix);

		uint32_t GetParticleCount() const { return countNum_; }
		SHEngine::GPUBuffer* GetIndexList() const { return indexList_; }
		SHEngine::GPUBuffer* GetCountBuffer() const { return countBuffer_; }

	private:

		Data* data_;

		SHEngine::GPUBuffer* indexList_;
		SHEngine::GPUBuffer* countBuffer_;
		SHEngine::GPUBuffer* emitterID_;

		std::map<uint32_t, std::unique_ptr<SHEngine::ComputeObject>> processes_;
		std::vector<std::unique_ptr<SHEngine::ComputeObject>> triggerProcesses_;

		uint32_t nextProcessID_ = 0;

		const uint32_t countNum_;
		const uint32_t processorKey_;
		const uint32_t id_;

		Processor* processor_ = nullptr;
	};

}
