#include "SRVManager.h"

SRVManager::SRVManager(DXDevice* device, int num) : maxCount(num), descriptorSizeSRV(device->GetDescriptorSizeSRV()) {

	srvDescriptorHeap.Attach(CreateDescriptorHeap(device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, maxCount, true));

	isUsed_.resize(maxCount, false);
}

uint32_t SRVManager::GetNextOffset(int startIndex) {
	uint32_t i = startIndex;

	for (i; i < maxCount; ++i) {
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
	if(isUpdated_)
		return;

	if (operateIndex == -1) {
		operateIndex = 1024;
	}

	offset_ = manager->GetNextOffset(operateIndex);
	manager_ = manager;
	CPU = GetCPUDesscriptorHandle(manager->GetHeap(), manager->descriptorSizeSRV, offset_);
	GPU = GetGPUDesscriptorHandle(manager->GetHeap(), manager->descriptorSizeSRV, offset_);
	manager_->isUsed_[offset_] = true;

	isUpdated_ = true;
}
