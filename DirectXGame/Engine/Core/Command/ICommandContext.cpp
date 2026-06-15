#include "ICommandContext.h"

using namespace SHEngine;

void SHEngine::ICommandContext::BeginTimeStamp(std::string name) {
	auto cmdObj = GetCurrentCmdObj();
	measureShaderTime_->Begin(cmdObj, name);
}

void SHEngine::ICommandContext::EndTimeStamp() {
	auto cmdObj = GetCurrentCmdObj();
	measureShaderTime_->End(cmdObj);
}

double SHEngine::ICommandContext::GetTimeStampResult(std::string name) {
	auto cmdObj = GetCurrentCmdObj();
	return measureShaderTime_->GetTimeStampResult(name);
}

void ICommandContext::PrivateInitialize(DXDevice* device, Command::Type type, int initCmdObjNum) {
	device_ = device;
	type_ = type;

	queue_ = std::make_unique<Command::Queue>(device, type);

	//必ず一つ以上は作成する
	initCmdObjNum = std::max(2, initCmdObjNum);

	cmdObjects_.reserve(initCmdObjNum);
	for (int i = 0; i < initCmdObjNum; i++) {
		cmdObjects_.push_back(std::make_unique<Command::Object>(device, type, 3));
	}

	lastWaitFence_.resize(device->GetBufferCount());

	measureShaderTime_ = std::make_unique<MeasureShaderTime>();
	measureShaderTime_->Initialize(device, queue_->GetQueue());
}

CmdObj* SHEngine::ICommandContext::GetCurrentCmdObj() {
	auto cmdObj = cmdObjects_[currentCmdObjIndex_].get();
	cmdObj->ResetCommandList();
	return cmdObj;
}

void SHEngine::ICommandContext::BeginFrame() {
	int currentCmdListIndex = cmdObjects_[currentCmdObjIndex_]->GetCurrentID();
	queue_->WaitFenceInCPU(lastWaitFence_[currentCmdListIndex]);

	auto cmdObj = GetCurrentCmdObj();
	measureShaderTime_->NewFrame(cmdObj);
}

Command::WaitFence SHEngine::ICommandContext::MiddleExecute() {
	// コマンドオブジェクトを実行して、フェンスを取得する
	auto cmdObj = GetCurrentCmdObj();
	auto fence = queue_->Execute({ cmdObj });

	// コマンドオブジェクトを次のものに切り替える
	currentCmdObjIndex_++;
	if (currentCmdObjIndex_ >= cmdObjects_.size()) {
		cmdObjects_.emplace_back(std::make_unique<Command::Object>(device_, type_, 3));
	}

	return fence;
}

void SHEngine::ICommandContext::EndFrame() {
	auto cmdObj = GetCurrentCmdObj();
	measureShaderTime_->FinFrame(cmdObj);

	auto fence = MiddleExecute();

	uint32_t cmdListIndex = cmdObjects_[currentCmdObjIndex_]->GetCurrentID();
	lastWaitFence_[cmdListIndex] = fence;

	currentCmdObjIndex_ = 0;
}
