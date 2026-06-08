#pragma once
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

namespace SHEngine {

	class DirectCommandContext {
	public:

		void Initialize(DXDevice* device);

		CmdObj* GetCurrentCmdObj() { return cmdObjects_[currentCmdObjIndex_].get(); }
		Command::WaitFence Execute();

	private:

		std::unique_ptr<Command::Queue> queue_ = nullptr;

		std::vector<std::unique_ptr<Command::Object>> cmdObjects_;
		std::vector<Command::WaitFence> waitFences_ = {};

		int currentCmdObjIndex_ = 0;

		std::vector<Command::WaitFence> lastWaitFence_ = {};

	};
}
