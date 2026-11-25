#include "DeferredReleaseQueue.h"
#include <cassert>

SHEngine::DeferredReleaseQueue::DeferredReleaseQueue(uint32_t waitFrames) : waitFrames_(waitFrames) {
	assert(instance_ == nullptr && "DeferredReleaseQueue: instance already exists");
	instance_ = this;
}

SHEngine::DeferredReleaseQueue::~DeferredReleaseQueue() {
	ReleaseAll();
	if (instance_ == this) {
		instance_ = nullptr;
	}
}

void SHEngine::DeferredReleaseQueue::Push(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>&& resources,
	std::vector<std::unique_ptr<SRVHandle>>&& descriptors) {
	if (resources.empty() && descriptors.empty()) {
		return;
	}
	Pending& pending = pending_.emplace_back();
	pending.remainFrames = waitFrames_;
	pending.resources = std::move(resources);
	pending.descriptors = std::move(descriptors);
}

void SHEngine::DeferredReleaseQueue::Update() {
	for (size_t i = 0; i < pending_.size();) {
		if (pending_[i].remainFrames <= 1) {
			// ディスクリプタ(SRVHandle)の破棄でスロットが空きに戻る
			pending_.erase(pending_.begin() + i);
		} else {
			--pending_[i].remainFrames;
			++i;
		}
	}
}

void SHEngine::DeferredReleaseQueue::ReleaseAll() {
	pending_.clear();
}
