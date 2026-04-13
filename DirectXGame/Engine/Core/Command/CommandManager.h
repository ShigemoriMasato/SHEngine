#pragma once
#include <Core/DXDevice.h>
#include <Tool/Logger/Logger.h>
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

//コマンドオブジェクトのエイリアス
using CmdObj = SHEngine::Command::Object;

namespace SHEngine::Command {

	class Manager {
	private:
#define QueueChecker assert(queue_.find(type) != queue_.end() && index < queue_[type].size())

	public:

		void Initialize(DXDevice* device);
		
		/// @brief コマンドオブジェクトを生成する
		/// @param type コマンドキューのタイプ
		/// @param index コマンドキューのインデックス
		std::unique_ptr<Object> CreateCommandObject(Type type, int index = 0, int listNum = 3);

		/// @brief コマンドを実行する
		/// @param type コマンドキューのタイプ
		/// @param index コマンドキューのインデックス
		void Execute(Type type, int index = 0, std::vector<CmdObj*> cmdObj = {});

		/// @brief コマンドオブジェクトを解放するとき、Managerからも削除する
		void ReleaseObject(Queue* queue, Object* obj);

		void StopGPU(Type type, int index = 0) {
			QueueChecker;
			queue_[type][index]->StopGPU();
		}

		ID3D12CommandQueue* GetCommandQueue(Type type, int index = 0) {
			QueueChecker;
			return queue_[type][index]->GetQueue();
		}

	private:

		DXDevice* device_;
		Logger logger_;

		std::map<Type, std::vector<std::unique_ptr<Queue>>> queue_{};
		std::map<Type, std::vector<std::vector<Object*>>> objects_{};
	};
}
