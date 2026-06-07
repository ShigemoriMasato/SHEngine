#pragma once
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

namespace SHEngine {

	class DirectCommandContext {
	public:

		void Initialize(DXDevice* device);

		Command::WaitFence Execute(std::vector<Command::Object*> cmdObjs = {});

	private:

		std::unique_ptr<Command::Queue> queue_ = nullptr;

		std::vector<std::unique_ptr<Command::Object>> cmdObjects_;

	};
}
