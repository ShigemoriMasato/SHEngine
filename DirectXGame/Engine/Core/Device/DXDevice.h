#pragma once
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Logger/Logger.h>

#include <wrl.h>


class DXDevice {
public:

	DXDevice() = default;
	~DXDevice();

	void Initialize();

	ID3D12Device* GetDevice() { return device_.Get(); }
	IDXGIFactory7* GetDxgiFactory() { return dxgiFactory_.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_.Get(); }

	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	uint32_t GetDescriptorSizeRTV() { return descriptorSizeRTV; }
	uint32_t GetDescriptorSizeDSV() { return descriptorSizeDSV; }

	void Execute(ID3D12CommandList** commandLists);
	void SendSignal();
	void WaitSignal();

private:

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	HANDLE fenceEvent_;
	uint32_t fenceValue_[2] = { 0,1 };

	int index_ = 0;

private:

	std::shared_ptr<spdlog::logger> logger_;

};
