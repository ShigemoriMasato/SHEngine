#pragma once
#include <Core/DXDevice.h>
#include <Tool/Logger/Logger.h>
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

namespace SHEngine::Command {

	class Manager {
	public:

		void Initialize(DXDevice* device);
		
		/// @brief コマンドオブジェクトを生成する
		/// @param type コマンドキューのタイプ
		/// @param index コマンドキューのインデックス
		std::unique_ptr<Object> CreateCommandObject(Type type, int listNum = 3);

		/// @brief コマンドを実行する
		/// @param type コマンドキューのタイプ
		/// @param index コマンドキューのインデックス
		WaitFence Execute(Type type, std::vector<CmdObj*> cmdObj = {});

		/// @brief コマンドオブジェクトを解放するとき、Managerからも削除する
		void ReleaseObject(Queue* queue, Object* obj);

		void WaitFence(const WaitFence& waitFence, Type type);

		void StopGPU(Type type) {
			QueueChecker(type);
			queue_[type]->StopGPU();
		}

		ID3D12CommandQueue* GetCommandQueue(Type type) {
			QueueChecker(type);
			return queue_[type]->GetQueue();
		}

	private:

		void QueueChecker(Type type);

		DXDevice* device_;
		Logger logger_;

		std::map<Type, std::unique_ptr<Queue>> queue_{};
		std::map<Type, std::vector<Object*>> objects_{};
	};
}
