#include "BufferContainer.h"

void SHEngine::BufferContainer::Erase(GPUBuffer* buffer) {
	std::unique_ptr<GPUBuffer> eraseBuffer;
	for (auto& b : buffers_) {
		if (b.get() == buffer) {
			eraseList_.emplace_back(0, std::move(eraseBuffer));
			return;
		}
	}
	assert(false && "BufferContainer::Erase: buffer not found");
}

void SHEngine::BufferContainer::EraseListUpdate() {
	for (int i = 0; i < (int)eraseList_.size(); ++i) {
		auto& [frame, buffer] = eraseList_[i];
		frame++;
		if (frame >= 3) {
			eraseList_.erase(eraseList_.begin() + i);
		}
	}
}
