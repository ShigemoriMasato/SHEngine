#pragma once
#include "Data/Function.h"

class SRVManager;

class SRVHandle final {
public:
	SRVHandle() = default;
	~SRVHandle();

	void UpdateHandle(SRVManager* manager, int operateIndex = -1);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return CPU; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return GPU; }
	int GetOffset() const { return offset_; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE CPU;
	D3D12_GPU_DESCRIPTOR_HANDLE GPU;
	int offset_;

	SRVManager* manager_;

	bool isUpdated_ = false;
};

class SRVManager {
public:

	SRVManager(DXDevice* device, int num);
	~SRVManager() = default;

	ID3D12DescriptorHeap* GetHeap() { return srvDescriptorHeap.Get(); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetStartPtr() { return srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

	uint32_t GetNextOffset(int startIndex);

private:

	friend class SRVHandle;

	//SRVHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	std::vector<bool> isUsed_;

	const uint32_t descriptorSizeSRV = 0;
	const uint32_t maxCount;

};
