#include "BufferContainer.h"

SHEngine::BufferContainer::~BufferContainer() {
	/*while (!buffers_.empty()) {
		eraseList_.emplace_back(0, std::move(buffers_.back()));
	}*/
}

void SHEngine::BufferContainer::Erase(GPUBuffer*& buffer) {
	for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
		if (it->get() == buffer) {
			// GPUがまだ使用している可能性があるので、解放待ちリストに移してから消す
			eraseList_.emplace_back(0, std::move(*it));
			buffers_.erase(it);
			buffer = nullptr;
			return;
		}
	}
	assert(false && "BufferContainer::Erase: buffer not found");
}

void SHEngine::BufferContainer::EraseListUpdate() {
	for (int i = 0; i < (int)eraseList_.size();) {
		auto& [frame, buffer] = eraseList_[i];
		frame++;
		if (frame >= 3) {
			eraseList_.erase(eraseList_.begin() + i);
		} else {
			++i;
		}
	}
}
