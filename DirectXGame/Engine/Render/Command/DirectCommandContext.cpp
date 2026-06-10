#include "DirectCommandContext.h"
#include "DirectCommandContext.h"

using namespace SHEngine;

void DirectCommandContext::Initialize(DXDevice* device, int initCmdObjNum) {
	queue_ = std::make_unique<Command::Queue>(device, Command::Type::Direct);

	//必ず一つ以上は作成する
	initCmdObjNum = std::max(1, initCmdObjNum);

	cmdObjects_.reserve(initCmdObjNum);
	for (int i = 0; i < initCmdObjNum; i++) {
		cmdObjects_.push_back(std::make_unique<Command::Object>(device, Command::Type::Direct, 3));
	}
}

void SHEngine::DirectCommandContext::BeginFrame() {
	currentCmdObjIndex_ = 0;
	cmdObjects_[currentCmdObjIndex_]->ResetCommandList();
}

Command::WaitFence SHEngine::DirectCommandContext::MiddleExecute() {
	auto cmdObj = GetCurrentCmdObj();

	// コマンドオブジェクトを次のものに切り替える
	currentCmdObjIndex_++;
	if (currentCmdObjIndex_ >= cmdObjects_.size()) {
		cmdObjects_.emplace_back(std::make_unique<Command::Object>(device_, Command::Type::Direct, 3));
	}
	cmdObjects_[currentCmdObjIndex_]->ResetCommandList();

	// コマンドオブジェクトを実行して、フェンスを取得する
	cmdObj->Close();
	auto cmdList = cmdObj->GetCommandList();
	return queue_->Execute({ cmdObj });
}

void SHEngine::DirectCommandContext::EndFrame() {
	auto fence = MiddleExecute();

	uint32_t cmdListIndex = cmdObjects_[currentCmdObjIndex_]->GetCurrentID();
	lastWaitFence_[cmdListIndex] = fence;
}
