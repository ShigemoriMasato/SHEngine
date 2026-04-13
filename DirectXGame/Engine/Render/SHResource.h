#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandManager.h>

enum class BufferType : uint8_t {
	CBV = 1 << 0,
	SRV = 1 << 1,
	UAV = 1 << 2,
};

BufferType operator|(BufferType a, BufferType b);
BufferType operator|(uint8_t a, BufferType b);
uint8_t operator&(uint8_t a, BufferType b);
uint8_t operator~(BufferType a);

enum class BufferUsage : uint8_t {
	DEFAULT,
	UPLOAD,
	READBACK,
};

namespace SHEngine {

	struct ResourceDesc {
		uint8_t bufferType;
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
		size_t sizeInBytes = 0;
		uint32_t elementCount = 0;
		uint32_t bufferNum = 3;		//バッファ数（SwapChain対策用）
	};

	class GPUBuffer {
	public:

		static void SetDevice(DXDevice* device) { device_ = device; }

		GPUBuffer(ResourceDesc& desc);

		void CopyBuffer(const void* data, size_t dataSize);
		void TransitionBarrier(D3D12_RESOURCE_STATES after);
		void Flush(CmdObj* cmdObj, uint32_t bufferIndex);

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(BufferType type, uint32_t bufferIndex) const;

	private:

		static inline DXDevice* device_ = nullptr;

		struct Resource {
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		std::vector<Resource> resources_ = {};

		std::map<BufferType, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> descriptorHandles_;
		std::vector<std::unique_ptr<SRVHandle>> srvHandles_;
		std::vector<std::unique_ptr<SRVHandle>> uavHandles_;

		size_t sizeInBytes_ = 0;
		std::vector<void*> mappedData_ = {};

		std::vector<D3D12_RESOURCE_STATES> currentState_ = {};

		//Flush時に切り替える用
		std::vector<uint8_t> nextData_;
		D3D12_RESOURCE_STATES nextState_ = {};
	};

}
