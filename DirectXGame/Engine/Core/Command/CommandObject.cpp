#include "CommandObject.h"
#include <Render/Screen/IDisplay.h>
#include <Utility/DirectUtilFuncs.h>

using namespace SHEngine::Command;

SHEngine::Command::Object::Object(DXDevice* device, Type type, int listNum) {
	device_ = device;

	if (listNum <= 0) {
		assert(false && "CommandObjectのコマンドリストの数は1以上でなければなりません。");
		listNum = 1;
	}

	// コマンドリストを作成
	commandLists_.resize(listNum);
	for (auto& cmdList : commandLists_) {
		cmdList = std::make_unique<DXList>();
		cmdList->Initialize(device_, type);
	}

	type_ = type;
}

Object::~Object() {
	WaitForAllCommandListIdle(); // すべてのコマンドが終了されるのを待つ
}

bool Object::CanExecute() {
	// 現在のコマンドリストが実行可能かどうかを確認
	return commandLists_[currentIndex_ % uint32_t(commandLists_.size())]->CanExecute();
}

void SHEngine::Command::Object::WaitForAllCommandListIdle() {
	for (auto& cmdList : commandLists_) {
		cmdList->WaitFenceInCPU();
	}
}

void SHEngine::Command::Object::ResetCommandList() {
	if (state_ == State::Open) {
		// コマンドリストが開いている場合はリセットするとエラーになるのでリセットしない
		return;
	}

	WaitFenceInCPU();

	commandLists_[currentIndex_ % uint32_t(commandLists_.size())]->ResetCommandList();
	state_ = State::Open;
}

void SHEngine::Command::Object::Execute(std::vector<ID3D12CommandList*>& cmdLists) {
	if (state_ == State::Close) {
		//実行済みの状態であるため、関数を終了させる
		return;
	}

	Close();

	uint32_t index = currentIndex_ % uint32_t(commandLists_.size());
	commandLists_[index]->Execute(cmdLists);

	state_ = State::Close;
}

void SHEngine::Command::Object::SetFence(WaitFence fence) {
	commandLists_[currentIndex_ % uint32_t(commandLists_.size())]->SetFence(fence);
}

void SHEngine::Command::Object::WaitFenceInCPU() {
	commandLists_[currentIndex_ % uint32_t(commandLists_.size())]->WaitFenceInCPU();
}

void SHEngine::Command::Object::Close() {
	if (state_ == State::Close) {
		//すでにクローズされている状態であるため、関数を終了させる
		return;
	}
	commandLists_[currentIndex_ % uint32_t(commandLists_.size())]->GetCommandList()->Close();
	state_ = State::Close;
}

std::string SHEngine::Command::Object::Log() const {
	std::string ans;
	ans = "CommandObject - Type: " + std::to_string(static_cast<int>(type_)) +
		", CurrentIndex: " + std::to_string(currentIndex_ % uint32_t(commandLists_.size()));
	return ans;
}
