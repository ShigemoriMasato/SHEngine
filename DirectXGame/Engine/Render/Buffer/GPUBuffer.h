#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>
#include <Assets/Texture/TextureData.h>
#include <Render/PSO/Shelf/RootSignatureShelf.h>

using RegisterCounter = std::unordered_map<ShaderType, std::unordered_map<BufferType, int>>;

enum class BufferNum : uint8_t {
	MatchSwapChain,
	Single,
	Double,
	Triple,
};

namespace SHEngine {

	class GPUBuffer {
	public:

		static void SetDevice(DXDevice* device) { device_ = device; }

		// @brief GPUBufferの作成
		// @param bufferType バッファの種類（CBV、SRV、UAVの組み合わせ）
		// @param size バッファのサイズ（バイト単位）
		// @param num バッファの数（デフォルトは1）
		// @param bufferNum バッファの数（デフォルトは3、スワップチェーンのバッファ数に合わせる）
		GPUBuffer(BufferType bufferType, size_t strideInBytes, uint32_t num = 1, BufferNum bufferNum = BufferNum::MatchSwapChain);

		// @brief Texture用のGPUBufferの作成
		GPUBuffer(TextureData* textureData);

		// @brief GPUBufferへデータコピーをするときの値を変更する。Flush時に実際にGPUへコピーされる。UAVバッファにはコピーできない。
		virtual void CopyBuffer(const void* data, size_t dataSize);
		// @brief GPUBufferのリソースバリアを設定する。Flush時に切り替える。
		void TransitionBarrier(ShaderType shaderType, BufferType bufferType);
		// @brief GPUBufferのリソースバリアを設定する。Flush時に切り替える。
		void TransitionBarrier(D3D12_RESOURCE_STATES nextState);
		// @brief GPUBufferの状態を実際にGPUへ反映させる。
		void Flush(ID3D12GraphicsCommandList* cmdList);

		// @brief GPUBufferのGPUディスクリプタハンドルを取得する。
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(BufferType type) const;

		// @brief Viewの種類を取得する
		uint8_t GetBufferType() const { return bufferType_; }

		ID3D12Resource* GetResource() const {
			if (resources_.empty()) return nullptr;
			return resources_[currentIndex_ % resources_.size()].res.Get();
		}

		//全体のサイズ
		size_t GetSizeInBytes() const { return sizeInBytes_; }
		//1要素のサイズ
		size_t GetStrideInBytes() const { return strideInBytes_; }
		uint32_t GetNum() const { return num_; }

	protected:

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
		size_t strideInBytes_ = 0;
		uint32_t num_ = 0;
		std::vector<void*> mappedData_ = {};

		std::vector<D3D12_RESOURCE_STATES> currentState_ = {};

		//Flush時に切り替える用
		std::vector<uint8_t> nextData_;
		D3D12_RESOURCE_STATES nextState_ = {};


		static inline uint32_t currentIndex_ = 0;
	};

}
