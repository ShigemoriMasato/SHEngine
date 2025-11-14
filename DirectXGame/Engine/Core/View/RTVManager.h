#pragma once
#include "Data/Function.h"

class RTVManager;

class RTVHandle final {
public:
	RTVHandle() = default;
	~RTVHandle();

	void UpdateHandle(RTVManager* manager, int operateIndex = 0);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPU() const { return CPU; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPU() const { return GPU; }
	int GetOffset() const { return offset_; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE CPU;
	D3D12_GPU_DESCRIPTOR_HANDLE GPU;
	int offset_;

	RTVManager* manager_;

	bool isUpdated_ = false;
};

class RTVManager {
public:

	RTVManager(DXDevice* device, int num);
	~RTVManager() = default;

	ID3D12DescriptorHeap* GetHeap() { return srvDescriptorHeap.Get(); }

	D3D12_GPU_DESCRIPTOR_HANDLE GetStartPtr() { return srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

	uint32_t GetNextOffset(int startIndex);

private:

	friend class RTVHandle;

	//RTVHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	std::vector<bool> isUsed_;

	const uint32_t descriptorSizeRTV = 0;
	const uint32_t maxCount;

};


