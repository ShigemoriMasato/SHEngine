#include "SHCmdQueue.h"

SHEngine::Command::Queue::Queue(DXDevice* device, Type type) {
	//Queueの作成
	D3D12_COMMAND_QUEUE_DESC desc{};

	switch (type) {
	case Type::Direct:
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		break;
	case Type::Compute:
		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		break;
	}
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
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

void SHEngine::Command::Queue::RegisterObject(Object* object) {
	objects_.push_back(object);
}

SHEngine::Command::WaitFence SHEngine::Command::Queue::Execute(std::vector<Object*> cmdObjs) {
	std::vector<ID3D12CommandList*> cmdLists;
	if (cmdObjs.empty()) {
		for (const auto& obj : objects_) {
			obj->Execute(cmdLists);
		}
	} else {
		for(const auto& obj : cmdObjs) {
			obj->Execute(cmdLists);
		}
	}

	commandQueue_->ExecuteCommandLists(UINT(cmdLists.size()), cmdLists.data());
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);

	WaitFence result = {};
	result.fence = fence_.Get();
	result.value = fenceValue_;
	return result;
}

uint64_t SHEngine::Command::Queue::GetLastSendFence() {
	return fenceValue_;
}

bool SHEngine::Command::Queue::CheckFinishedJob(uint64_t fenceValue) {
	return fence_->GetCompletedValue() < fenceValue;
}

void SHEngine::Command::Queue::WaitForFence(uint64_t fence) {
	uint64_t currentFence = fence_->GetCompletedValue();
	if (currentFence < fence) {
		//完了していなければイベントをセットして待機
		HRESULT hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		assert(SUCCEEDED(hr) && "Failed to set event on completion");
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

void SHEngine::Command::Queue::WaitForFence(const WaitFence& waitFence) {
	commandQueue_->Wait(waitFence.fence, waitFence.value);
}

void SHEngine::Command::Queue::StopGPU() {
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);
	WaitForFence(fenceValue_);
}
