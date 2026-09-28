#include "SRVManager.h"
#include <stdexcept>

SRVManager::SRVManager(ID3D12Device* device, uint32_t size, int num) : maxCount_(num), descriptorSizeSRV(size) {

	heap_.Attach(CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, maxCount_, true));

	isUsed_.resize(maxCount_, false);
}

uint32_t SRVManager::GetNextOffset(int startIndex) {
	uint32_t i = startIndex;

	if(i >= maxCount_) {
		i = 0;
	}

	for (i; i < maxCount_; ++i) {
		if (!isUsed_[i]) {
			return i;
		}
	}

	throw std::runtime_error("No available SRV descriptor slots.");
}

SRVHandle::~SRVHandle() {
	if (manager_)
		manager_->isUsed_[offset_] = false;
}

void SRVHandle::UpdateHandle(SRVManager* manager, int operateIndex) {
	if (isUpdated_)
		return;

	if (operateIndex == -1) {
		operateIndex = manager->maxCount_ / 2;
	}

	offset_ = manager->GetNextOffset(operateIndex);
	manager_ = manager;
	CPU = GetCPUDescriptorHandle(manager->GetHeap(), manager->descriptorSizeSRV, offset_);
	GPU = GetGPUDescriptorHandle(manager->GetHeap(), manager->descriptorSizeSRV, offset_);
	manager->isUsed_[offset_] = true;

	isUpdated_ = true;
}
