#include "DirectCommandContext.h"
#include "DirectCommandContext.h"

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	queue_ = std::make_unique<Command::Queue>(device, Command::Type::Direct);

	//必ず一つ以上は作成する
	initCmdObjNum = std::max(2, initCmdObjNum);

	cmdObjects_.reserve(initCmdObjNum);
	for (int i = 0; i < initCmdObjNum; i++) {
		cmdObjects_.push_back(std::make_unique<Command::Object>(device, Command::Type::Direct, 3));
	}

	lastWaitFence_.resize(3);

	device_ = device;
}

CmdObj* SHEngine::DirectCommandContext::GetCurrentCmdObj() {
	auto cmdObj = cmdObjects_[currentCmdObjIndex_].get();
	cmdObj->ResetCommandList();
	return cmdObj;
}

void SHEngine::DirectCommandContext::BeginFrame() {
	int currentCmdListIndex = cmdObjects_[currentCmdObjIndex_]->GetCurrentID();
	queue_->WaitFenceInCPU(lastWaitFence_[currentCmdListIndex]);
}

Command::WaitFence SHEngine::DirectCommandContext::MiddleExecute() {
	// コマンドオブジェクトを実行して、フェンスを取得する
	auto cmdObj = GetCurrentCmdObj();
	auto fence = queue_->Execute({ cmdObj });

	// コマンドオブジェクトを次のものに切り替える
	currentCmdObjIndex_++;
	if (currentCmdObjIndex_ >= cmdObjects_.size()) {
		cmdObjects_.emplace_back(std::make_unique<Command::Object>(device_, Command::Type::Direct, 3));
	}
	cmdObjects_[currentCmdObjIndex_]->ResetCommandList();

	return fence;
}

void SHEngine::DirectCommandContext::EndFrame() {
	auto fence = MiddleExecute();

	uint32_t cmdListIndex = cmdObjects_[currentCmdObjIndex_]->GetCurrentID();
	lastWaitFence_[cmdListIndex] = fence;

	currentCmdObjIndex_ = 0;
}
