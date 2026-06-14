#include "CommandSet.h"
#include "SHCmdQueue.h"

using namespace SHEngine::Command;

SHEngine::Command::DXList::~DXList() {
	WaitFenceInCPU();
}

void DXList::Initialize(DXDevice* device, Type type) {
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
	HRESULT hr = device->GetDevice()->CreateCommandAllocator(
		commandListType,
		IID_PPV_ARGS(&commandAllocator_)
	);
	assert(SUCCEEDED(hr) && "Failed to create Command Allocator");

	// コマンドリストの作成
	hr = device->GetDevice()->CreateCommandList(
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

	//リセット時にSRVヒープをセットするために保存しておく
	srvHeap_ = device->GetSRVManager()->GetHeap();
}

void SHEngine::Command::DXList::Execute(std::vector<ID3D12CommandList*>& cmdLists) {
	cmdLists.push_back(commandList_.Get());
}

bool SHEngine::Command::DXList::CanExecute() {
	if (!currentFence_.fence) {
		return true;
	}

	return currentFence_.fence->GetCompletedValue() >= currentFence_.value;
}

void SHEngine::Command::DXList::WaitFenceInCPU() {
	if (CanExecute()) {
		return;
	}

	HRESULT hr = currentFence_.fence->SetEventOnCompletion(currentFence_.value, currentFence_.fenceEvent);
	assert(SUCCEEDED(hr));

	WaitForSingleObject(currentFence_.fenceEvent, INFINITE);
}

void DXList::ResetCommandList() {
	WaitFenceInCPU();

	//コマンドリストとアロケータをリセット
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr) && "Failed to reset Command Allocator");
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr) && "Failed to reset Command List");

	//Heapのセット
	commandList_->SetDescriptorHeaps(1, &srvHeap_);
}
