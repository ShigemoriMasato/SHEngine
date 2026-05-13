#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandManager.h>
#include <Assets/Texture/TextureData.h>

enum class BufferType : uint8_t {
	CBV = 1 << 0,
	SRV = 1 << 1,
	UAV = 1 << 2,

	CBV_SRV = 0b011,
	//CBV_UAV = 0b101,		使えないやつ
	SRV_UAV = 0b110,
	//CBV_SRV_UAV = 0b111,	使えないやつ

	Texture = 0b1000,
	Texture2D = 0b1001,
	DDSTexture = 0b1010,
};

uint8_t operator&(uint8_t a, BufferType b);
uint8_t operator~(BufferType a);

namespace SHEngine {

	class GPUBuffer {
	public:

		static void SetDevice(DXDevice* device) { device_ = device; }

		// @brief GPUBufferの作成
		// @param bufferType バッファの種類（CBV、SRV、UAVの組み合わせ）
		// @param size バッファのサイズ（バイト単位）
		// @param num バッファの数（デフォルトは1）
		// @param bufferNum バッファの数（デフォルトは3、スワップチェーンのバッファ数に合わせる）
		GPUBuffer(BufferType bufferType, size_t size, uint32_t num = 1, uint32_t bufferNum = 3);

		// @brief Texture用のGPUBufferの作成
		GPUBuffer(TextureData* textureData);

		// @brief GPUBufferへデータコピーをするときの値を変更する。Flush時に実際にGPUへコピーされる。UAVバッファにはコピーできない。
		void CopyBuffer(const void* data, size_t dataSize);
		// @brief GPUBufferのリソースバリアを設定する。Flush時に切り替える。
		void TransitionBarrier(D3D12_RESOURCE_STATES after);
		// @brief GPUBufferの状態を実際にGPUへ反映させる。
		void Flush(CmdObj* cmdObj);

		// @brief GPUBufferのGPUディスクリプタハンドルを取得する。
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(BufferType type) const;

		// @brief Viewの種類を取得する
		uint8_t GetBufferType() const { return bufferType_; }

	private:

		friend class FrameCounter;
		static void SetCurrentIndex(uint32_t frame) { currentIndex_ = frame; }

		static inline DXDevice* device_ = nullptr;

		struct Resource {
			Microsoft::WRL::ComPtr<ID3D12Resource> res;
		};
		std::vector<Resource> resources_ = {};

		uint8_t bufferType_;

		std::map<BufferType, std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> descriptorHandles_;
		std::vector<std::unique_ptr<SRVHandle>> srvHandles_;
		std::vector<std::unique_ptr<SRVHandle>> uavHandles_;

		size_t sizeInBytes_ = 0;
		std::vector<void*> mappedData_ = {};

		std::vector<D3D12_RESOURCE_STATES> currentState_ = {};

		//Flush時に切り替える用
		std::vector<uint8_t> nextData_;
		D3D12_RESOURCE_STATES nextState_ = {};


		static inline uint32_t currentIndex_ = 0;
	};

}
