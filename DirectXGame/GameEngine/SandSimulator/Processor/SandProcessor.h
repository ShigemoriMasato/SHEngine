#pragma once
#include <SandSimulator/Data/SandData.h>
#include <Compute/ComputeObject.h>
#include <SandSimulator/Debugger/SandConfigure.h>
#include <Tool/Binary/BinaryManager.h>
#include <random>

namespace Sand {

	class Processor {
	public:

		Processor(SHEngine::BufferContainer* containe, Particle* particle, Grid* grid, Configure* configure);

		void Initialize(SHEngine::ICommandContext* cc, Vector3 pos, float radius);
		void Update(SHEngine::ICommandContext* cc, float deltaTime);

		void Save(BinaryManager* bin) const;
		void Load(BinaryManager* bin);

		int GetProcessCount() const { return processCount_; }

	private:

		std::unique_ptr<SHEngine::ComputeObject> ballShape_ = nullptr;

		std::unique_ptr<SHEngine::ComputeObject> frameInitialize_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> acceptForce_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> counter_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> splitBlock_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> prefixSum_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> assignCell_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> collider_ = nullptr;

		std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;

		SHEngine::GPUBuffer* deltaTime_ = nullptr;
		SHEngine::GPUBuffer* seed_ = nullptr;
		SHEngine::GPUBuffer* ballShapeData_ = nullptr;
		SHEngine::GPUBuffer* cellCountPerBlockBuffer_ = nullptr;

		SHEngine::GPUBuffer* forceData_ = nullptr;
		SHEngine::GPUBuffer* gravity_ = nullptr;

		Vector3 commonUpdateData_ = { 0.0f, 0.0f, 0.0f };
		Vector3 gravityData_ = { 0.0f, -9.8f, 0.0f };

		int processCount_ = 0;
		const int maxProcessCount_;

		int cellCountPerBlock_ = 128;

		std::mt19937 randomEngine_;
		std::uniform_int_distribution<uint32_t> seedGenerator_;
	};

}
