#include "SHCmdQueue.h"

SHEngine::Command::Queue::Queue(DXDevice* device, Type type) {
	//Queueの作成
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	switch (type) {
	case Type::Direct:
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;	// 描画コマンドは優先度を高くする
		break;
	case Type::Compute:
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	}
	HRESULT hr = device->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr) && "Failed to create CommandQueue");

	// フェンスの作成
	hr = device->GetDevice()->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr) && "Failed to create Fence");
	// フェンスイベントの作成
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ && "Failed to create Fence Event");
}

SHEngine::Command::Queue::~Queue() {
	// フェンスイベントのクローズ
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}

SHEngine::Command::WaitFence SHEngine::Command::Queue::Execute(std::vector<Object*> cmdObjs) {
	std::vector<ID3D12CommandList*> cmdLists;
	for (const auto& obj : cmdObjs) {
		obj->Execute(cmdLists);
	}

	commandQueue_->ExecuteCommandLists(UINT(cmdLists.size()), cmdLists.data());
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);

	WaitFence result = {};
	result.fence = fence_.Get();
	result.fenceEvent = fenceEvent_;
	result.value = fenceValue_;

	for (auto& obj : cmdObjs) {
		obj->SetFence(result);
	}

	return result;
}

uint64_t SHEngine::Command::Queue::GetLastSendFence() {
	return fenceValue_;
}

bool SHEngine::Command::Queue::CheckFinishedJob(uint64_t fenceValue) {
	return fence_->GetCompletedValue() < fenceValue;
}

void SHEngine::Command::Queue::WaitFenceInCPU(const WaitFence& fence) {
	if(!fence.fence || !fence.fenceEvent) {
		return;
	}

	if (!(fence.fence->GetCompletedValue() >= fence.value)) {
		return;
	}

	HRESULT hr = fence.fence->SetEventOnCompletion(fence.value, fence.fenceEvent);
	assert(SUCCEEDED(hr));

	WaitForSingleObject(fence.fenceEvent, INFINITE);
}

void SHEngine::Command::Queue::WaitFenceInGPU(const WaitFence& fence) {
	commandQueue_->Wait(fence.fence, fence.value - 1);
}

void SHEngine::Command::Queue::StopGPU() {
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);
	WaitFence fence = { fence_.Get(), fenceEvent_, fenceValue_ };
	WaitFenceInCPU(fence);
}
