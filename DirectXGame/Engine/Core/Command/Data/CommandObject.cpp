#include "CommandObject.h"
#include <Core/Command/CommandManager.h>

using namespace SHEngine::Command;

SHEngine::Command::Object::Object(DXDevice* device, Manager* manager, Type type, Queue* queue, int listNum) {
	device_ = device;

	// コマンドリストを3つ作成
	commandLists_.resize(listNum);
	for (auto& cmdList : commandLists_) {
		cmdList.Initialize(device_, type);
	}

	// コマンドオブジェクトのタイプとキューインデックスを保存
	queue_ = queue;
	type_ = type;
	manager_ = manager;

	ResetCommandList();
}

Object::~Object() {
	WaitForGPUIdle(); // すべてのコマンドが終了されるのを待つ
	manager_->ReleaseObject(queue_, this); // Managerからも削除する
}

bool Object::CanExecute() {
	// 現在のコマンドリストが実行可能かどうかを確認
	return commandLists_[dxListIndex_].CanExecute();
}

void SHEngine::Command::Object::WaitForGPUIdle() {
	for(auto& cmdList : commandLists_) {
		cmdList.WaitForCanExecute();
	}
}

void SHEngine::Command::Object::ResetCommandList() {
	if(state_ == State::Open) {
		// コマンドリストが開いている場合はリセットするとエラーになるのでリセットしない
		return;
	}

	commandLists_[dxListIndex_].ResetCommandList();
	state_ = State::Open;
}

void SHEngine::Command::Object::Execute(std::vector<ID3D12CommandList*>& cmdLists) {
	if(state_ == State::Close) {
		//実行できるようにリセットする
		ResetCommandList();
	}

	commandLists_[dxListIndex_].Execute(queue_, cmdLists);

	state_ = State::Close;
}

std::string SHEngine::Command::Object::Log() const {
	std::string ans;
	ans = "CommandObject - Type: " + std::to_string(static_cast<int>(type_)) +
		", CurrentDXListIndex: " + std::to_string(dxListIndex_);
	return ans;
}
