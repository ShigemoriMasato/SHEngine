#pragma once
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

namespace SHEngine {

	class DirectCommandContext {
	public:

		void Initialize(DXDevice* device, int initCmdObjNum = 3);

		CmdObj* GetCurrentCmdObj() { return cmdObjects_[currentCmdObjIndex_].get(); }

		/// @brief コマンドを積むためのコマンドオブジェクトを準備する
		void BeginFrame();

		/// @brief コマンドを実行して、GPUが処理を終えるのを待つためのフェンスを返す
		Command::WaitFence GetFence();

		/// @brief コマンドを実行する
		void EndFrame();

	private:

		std::unique_ptr<Command::Queue> queue_ = nullptr;

		std::vector<std::unique_ptr<Command::Object>> cmdObjects_;

		int currentCmdObjIndex_ = 0;

		std::vector<Command::WaitFence> lastWaitFence_ = {};

		DXDevice* device_ = nullptr;

	};
}
