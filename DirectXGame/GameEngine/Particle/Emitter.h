#pragma once
#include <Compute/ComputeObject.h>
#include <Particle/System/ParticleData.h>
#include <Particle/System/ParticleProcessor.h>
#include <Compute/Graph/ComputeTrigger.h>
#include <map>

namespace Particle {

	class Emitter {
	public:

		// @param name エミッターの名前。ComputeGraphの名前("Particle/<name>/<Trigger名>")やResourceウィンドウのグループ名になる
		Emitter(uint32_t emitterID, uint32_t particleCount, Data* data, FreeList* freeList, Processor* processor, ICC* icc, SHEngine::BufferContainer* container, const std::string& name);

		// ---- ComputeGraph ----
		// 作成したComputeTriggerには、AddProcessと同じ割り当てが既定として設定される。
		//   b0: ParticleCount / t0: IndexList / u0: Positions / u1: Rotations / u2: Scales / u3: Colors
		// 同じ名前の引数としてもBindされ、数 "Particles" (このエミッターのパーティクル数) もSetCountされる。
		// それ以外のバッファは、返されたComputeTriggerにBindする。

		// @brief ComputeGraphを実行するComputeTriggerを作成する。グラフ名は "Particle/<エミッター名>/<triggerName>"
		// @return 作成した(作成済みならそれを返す)ComputeTrigger。エミッターが所有する
		SHEngine::ComputeTrigger* CreateTrigger(const std::string& triggerName);
		// @brief 作成済みのComputeTrigger。なければnullptr
		SHEngine::ComputeTrigger* GetTrigger(const std::string& triggerName) const;
		// @brief 毎フレームUpdateで実行するグラフを設定する(なければ作成する)
		SHEngine::ComputeTrigger* SetUpdateGraph(const std::string& triggerName);
		// @brief 次のUpdateで1回だけグラフを実行する(AddTriggerProcessのグラフ版)。Processorの準備前に呼んでもよい
		void Request(const std::string& triggerName);

		const std::string& GetName() const { return name_; }

		// @brief 毎フレーム実行する処理を追加
		// @param shaderPath Shader/Compute/Particle/以下を指定。.CS.hlslは不要
		// @param output 追加したComputeObjectのIDを返す。EraseProcessで削除する際に使用する。nullptrでは無視される
		// @return 追加したComputeObjectのポインタを返す。
		SHEngine::ComputeObject* AddProcess(std::string shaderPath, uint32_t* output = nullptr);

		// @brief 一度実行したら破棄される処理を追加
		// @param shaderPath Shader/Compute/Particle/以下を指定。.CS.hlslは不要
		// @return 追加したComputeObjectのポインタを返す。
		SHEngine::ComputeObject* AddTriggerProcess(std::string shaderPath);

		// @brief Managerで実行されず、自分のタイミングで実行する必要がある処理を追加
		// @param shaderPath Shader/Compute/Particle/以下を指定。.CS.hlslは不要
		// @return 追加したComputeObjectのポインタを返す。
		std::unique_ptr<SHEngine::ComputeObject> AddOprionalProcess(std::string shaderPath);

		// @param id AddProcessで使用したID。
		void EraseProcess(uint32_t id);

		// @param 実行順 [TriggerProcess -> Requestされたグラフ -> Process -> UpdateGraph]
		void Update(uint32_t processorKey, ICC* icc);

		void SetParentMatrix(const Matrix4x4& matrix);

		// デバッグ用。このエミッターが所有するすべてのBufferを取り出す。
		std::vector<SHEngine::GPUBuffer*> GetBuffer() const;

		uint32_t GetParticleCount() const { return countNum_; }
		SHEngine::GPUBuffer* GetIndexList() const { return isReleased_ ? nullptr : indexList_; }
		SHEngine::GPUBuffer* GetCountBuffer() const { return isReleased_ ? nullptr : countBuffer_; }

		bool IsReleased() const { return isReleased_; }

		void Release() { isReleased_ = true; };

	private:

		Data* data_;

		SHEngine::GPUBuffer* indexList_;
		SHEngine::GPUBuffer* countBuffer_;
		SHEngine::GPUBuffer* emitterID_;

		std::map<uint32_t, std::unique_ptr<SHEngine::ComputeObject>> processes_;
		std::vector<std::unique_ptr<SHEngine::ComputeObject>> triggerProcesses_;

		uint32_t nextProcessID_ = 0;

		// ComputeGraph
		std::string name_;
		std::map<std::string, std::unique_ptr<SHEngine::ComputeTrigger>> triggers_;
		SHEngine::ComputeTrigger* updateTrigger_ = nullptr;
		std::vector<SHEngine::ComputeTrigger*> requests_;

		const uint32_t countNum_;
		const uint32_t processorKey_;
		const uint32_t id_;

		Processor* processor_ = nullptr;

		bool isReleased_ = false;
	};

}
