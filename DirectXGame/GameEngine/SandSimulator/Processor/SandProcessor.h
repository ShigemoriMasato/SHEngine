#pragma once
#include <SandSimulator/Data/SandData.h>
#include <Compute/ComputeObject.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <Tool/Binary/BinaryManager.h>
#include <random>

namespace Sand {

	class Processor {
	public:

		Processor(SHEngine::BufferContainer* containe, Particle* particle, Grid* grid, ImGuiWindowManager* configure);

		void Initialize(SHEngine::ICommandContext* cc, Vector3 pos, float radius);
		void Update(SHEngine::ICommandContext* cc, float deltaTime);

		void Save(BinaryManager* bin) const;
		void Load(BinaryManager* bin);

		int GetProcessCount() const { return processCount_; }

	public:

		struct TimeConfig {
			double prefixSum = 0.0f;
		};

		TimeConfig GetTimeConfig() const { return timeConfig_; }

	private:

		std::unique_ptr<SHEngine::ComputeObject> ballShape_ = nullptr;

		std::unique_ptr<SHEngine::ComputeObject> frameInitialize_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> acceptForce_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> counter_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> splitBlock_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> prefixSum_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> assignCell_ = nullptr;
		std::unique_ptr<SHEngine::ComputeObject> colliderGo_ = nullptr;		//Subに書き込む側
		std::unique_ptr<SHEngine::ComputeObject> colliderBack_ = nullptr;	//Posに書き込む側

		std::unique_ptr<SHEngine::ComputeObject> initialize_ = nullptr;

		SHEngine::GPUBuffer* deltaTime_ = nullptr;
		SHEngine::GPUBuffer* seed_ = nullptr;
		SHEngine::GPUBuffer* ballShapeBuffer_ = nullptr;
		SHEngine::GPUBuffer* cellCountPerBlockBuffer_ = nullptr;
		SHEngine::GPUBuffer* colliderBuffer_ = nullptr;

		//当たり判定時、他のスレッドが読み取る際に書き込みが行われると正しい結果が得られない問題を解決するためのバッファ。
		SHEngine::GPUBuffer* subPosition_ = nullptr;

		SHEngine::GPUBuffer* forceData_ = nullptr;
		SHEngine::GPUBuffer* gravity_ = nullptr;

		Vector3 commonUpdateData_ = { 0.0f, 0.0f, 0.0f };
		Vector3 gravityData_ = { 0.0f, -9.8f, 0.0f };
		Vector2 colliderData_ = { 0.0f, 0.0f };

		int processCount_ = 0;
		const int maxProcessCount_;

		int cellCountPerBlock_ = 128;

		TimeConfig timeConfig_ = {};

		std::mt19937 randomEngine_;
		std::uniform_int_distribution<uint32_t> seedGenerator_;
	};

}
