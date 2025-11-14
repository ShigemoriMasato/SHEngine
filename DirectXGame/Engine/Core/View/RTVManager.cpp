#include "RTVManager.h"

RTVManager::RTVManager(DXDevice* device, int num) : maxCount(num), descriptorSizeRTV(device->GetDescriptorSizeRTV()) {

	srvDescriptorHeap.Attach(CreateDescriptorHeap(device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, maxCount, false));

	isUsed_.resize(maxCount, false);
}

uint32_t RTVManager::GetNextOffset(int startIndex) {
	uint32_t i = startIndex;

	for (i; i < maxCount; ++i) {
		if (!isUsed_[i]) {
			return i;
		}
	}

	throw std::runtime_error("No available RTV descriptor slots.");
}

RTVHandle::~RTVHandle() {
	if (manager_)
		manager_->isUsed_[offset_] = false;
}

void RTVHandle::UpdateHandle(RTVManager* manager, int operateIndex) {
	if (isUpdated_)
		return;

	offset_ = manager->GetNextOffset(operateIndex);
	manager_ = manager;
	CPU = GetCPUDesscriptorHandle(manager->GetHeap(), manager->descriptorSizeRTV, offset_);
	manager_->isUsed_[offset_] = true;

	isUpdated_ = true;
}
