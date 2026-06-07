#include "CommandSet.h"
#include "SHCmdQueue.h"

using namespace SHEngine::Command;

SHEngine::Command::DXList::~DXList() {
	WaitForCanExecute(); // コマンドリストが実行可能になるまで待機
}

void DXList::Initialize(DXDevice* device, Type type) {
	device_ = device;

	D3D12_COMMAND_LIST_TYPE commandListType{};

	switch (type) {
	case Type::Direct:
		commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case Type::Compute:
		commandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	}

	// コマンドアロケータの作成
	HRESULT hr = device_->GetDevice()->CreateCommandAllocator(
		commandListType,
		IID_PPV_ARGS(&commandAllocator_)
	);
	assert(SUCCEEDED(hr) && "Failed to create Command Allocator");

	// コマンドリストの作成
	hr = device_->GetDevice()->CreateCommandList(
		0,
		commandListType,
		commandAllocator_.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList_)
	);

	assert(SUCCEEDED(hr) && "Failed to create Command List");
	// コマンドリストは初期状態で記録モードなので閉じておく
	hr = commandList_->Close();
	assert(SUCCEEDED(hr) && "Failed to close Command List");
}

void SHEngine::Command::DXList::Execute(std::vector<ID3D12CommandList*>& cmdLists) {
	commandList_->Close();
	cmdLists.push_back(commandList_.Get());
}

void DXList::ResetCommandList() {
	WaitForCanExecute(); // コマンドリストが実行可能になるまで待機

	//コマンドリストとアロケータをリセット
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr) && "Failed to reset Command Allocator");
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr) && "Failed to reset Command List");

	//Heapのセット
	auto srvHeap = device_->GetSRVManager()->GetHeap();
	commandList_->SetDescriptorHeaps(1, &srvHeap);
}
