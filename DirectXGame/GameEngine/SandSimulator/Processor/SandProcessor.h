#pragma once
#include "../Data/SandData.h"
#include <Compute/ComputeObject.h>

namespace Sand {

	class Processor {
	public:

		Processor();

		void Initialize(Sand::Particle* particle, Sand::Grid* grid, Sand::FreeList* freeList, SHEngine::ICommandContext* icc);
		void Update(float deltaTime, SHEngine::ICommandContext* icc);

		void AddSand(const Sand::AddSandConfig& config, const SHEngine::ICommandContext* icc);
		void AddForce(const Sand::ShaderConfig& config, const SHEngine::ICommandContext* icc);

	private:

		std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;

		std::unique_ptr<SHEngine::ComputeObject> gridInitialize_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> sendDensity_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> addForce_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> particleMove_ = nullptr;

		std::unique_ptr<SHEngine::GPUBuffer> deltaTimeBuffer_ = nullptr;

		std::vector<std::unique_ptr<SHEngine::ComputeObject>> addOuterForce_ = {};

	};

}
