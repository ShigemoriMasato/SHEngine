#pragma once
#include "Data/Function.h"

class DSVManager;

class DSVHandle final {
public:
	DSVHandle() = default;
	~DSVHandle();

	void UpdateHandle(DSVManager* manager, int operateIndex = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return CPU; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return GPU; }
	int GetOffset() const { return offset_; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE CPU;
	D3D12_GPU_DESCRIPTOR_HANDLE GPU;
	int offset_;

	DSVManager* manager_;

	bool isUpdated_ = false;
};

class DSVManager {
public:

	DSVManager(DXDevice* device, int num);
	~DSVManager() = default;

	ID3D12DescriptorHeap* GetHeap() { return srvDescriptorHeap.Get(); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetStartPtr() { return srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

	uint32_t GetNextOffset(int startIndex);

private:

	friend class DSVHandle;

	//DSVHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	std::vector<bool> isUsed_;

	const uint32_t descriptorSizeDSV = 0;
	const uint32_t maxCount;

};


