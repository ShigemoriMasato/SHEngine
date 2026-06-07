#include "CommandObject.h"
#include <Core/Command/CommandManager.h>
#include <Render/Screen/IDisplay.h>
#include <Utility/DirectUtilFuncs.h>

using namespace SHEngine::Command;

SHEngine::Command::Object::Object(DXDevice* device, Type type, Queue* queue, int listNum) {
	device_ = device;

	if (listNum <= 0) {
		assert(false && "CommandObjectのコマンドリストの数は1以上でなければなりません。");
		listNum = 1;
	}

	// コマンドリストを作成
	commandLists_.resize(listNum);
	for (auto& cmdList : commandLists_) {
		cmdList.Initialize(device_, type);
	}

	// コマンドオブジェクトのタイプとキューインデックスを保存

	ResetCommandList();
}

Object::~Object() {
	WaitForGPUIdle(); // すべてのコマンドが終了されるのを待つ
}

bool Object::CanExecute() {
	// 現在のコマンドリストが実行可能かどうかを確認
	return commandLists_[currentIndex_ % uint32_t(commandLists_.size())].CanExecute();
}

void SHEngine::Command::Object::WaitForGPUIdle() {
	for (auto& cmdList : commandLists_) {
		cmdList.WaitForCanExecute();
	}
}

void SHEngine::Command::Object::SetRenderTarget(Screen::IDisplay* display, bool clear) {
	renderTarget_ = display;

	auto texture = display->GetTextureData();
	auto depthTexture = display->GetDepthTexture();
	auto rtvHandle = display->GetRTVHandle();
	auto dsvHandle = display->GetDSVHandle();
	auto cmdList = GetCommandList();

	display->ToRenderTarget(this);

	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//ViewPortとScissorRectの設定
	D3D12_VIEWPORT viewPort{};
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(texture->GetSize().first);
	viewPort.Height = static_cast<float>(texture->GetSize().second);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	cmdList->RSSetViewports(1, &viewPort);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(texture->GetSize().first);
	scissorRect.bottom = static_cast<LONG>(texture->GetSize().second);

	cmdList->RSSetScissorRects(1, &scissorRect);

	if (clear) {
		display->Clear(this);
	}
}

void SHEngine::Command::Object::ResetCommandList() {
	if (state_ == State::Open) {
		// コマンドリストが開いている場合はリセットするとエラーになるのでリセットしない
		return;
	}

	commandLists_[currentIndex_ % uint32_t(commandLists_.size())].ResetCommandList();
	state_ = State::Open;
}

void SHEngine::Command::Object::Execute(std::vector<ID3D12CommandList*>& cmdLists) {
	if (state_ == State::Close) {
		//実行できるようにリセットする
		ResetCommandList();
	}

	commandLists_[currentIndex_ % uint32_t(commandLists_.size())].Execute(queue_, cmdLists);

	state_ = State::Close;
}

std::string SHEngine::Command::Object::Log() const {
	std::string ans;
	bool currentCmdListState = commandLists_[currentIndex_ % uint32_t(commandLists_.size())].CanExecute();
	bool nextCmdListState = commandLists_[(currentIndex_ + 1) % uint32_t(commandLists_.size())].CanExecute();
	bool prevCmdListState = commandLists_[(currentIndex_ - 1) % uint32_t(commandLists_.size())].CanExecute();

	ans = "CommandObject - Type: " + std::to_string(static_cast<int>(type_)) +
		", CurrentIndex: " + std::to_string(currentIndex_ % uint32_t(commandLists_.size())) +
		", PrevState: " + (prevCmdListState ? "Finished" : "Processing") +
		", CurrentState: " + (currentCmdListState ? "Finished" : "Processing") +
		", NextState: " + (nextCmdListState ? "Finished" : "Processing");
	return ans;
}
