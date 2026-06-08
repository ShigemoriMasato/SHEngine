#include "DirectCommandContext.h"
#include "DirectCommandContext.h"

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device) {
	queue_ = std::make_unique<Command::Queue>(device, Command::Type::Direct);

	cmdObjects_.push_back(std::make_unique<Command::Object>(device, Command::Type::Direct));
}

Command::WaitFence DirectCommandContext::Execute() {
	auto cmdObj = GetCurrentCmdObj();
	cmdObj->Close();
}
