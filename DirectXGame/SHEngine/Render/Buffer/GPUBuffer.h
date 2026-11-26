#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>
#include <Render/PSO/Shelf/RootSignatureShelf.h>
#include <source_location>


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

		// @brief リソースとSRV/UAVのディスクリプタは、GPUが使用中かもしれないので解放待ちキューに預けて数フレーム後に解放する。
		//        エンジン終了後(キューがない)は即時解放する。
		virtual ~GPUBuffer();

		// @brief GPUBufferへデータコピーをするときの値を変更する。Flush時に実際にGPUへコピーされる。UAVバッファにはコピーできない。
		// @param location 呼び出し元(同一フレームでの使用後の書き換えを警告するために記録する)。通常は省略する
		// @note 同じフレームでFlush(使用)した後にもう一度CopyBufferすると、GPU上では先に使った箇所も後の値になるため警告を出す(Release以外)
		virtual void CopyBuffer(const void* data, size_t dataSize, std::source_location location = std::source_location::current());
		// @brief テクスチャ(TextureData派生)かどうか
		virtual bool IsTexture() const { return false; }

		// @brief 名前を設定する。ID3D12Resourceにも同じ名前が付く(フレームごとに複数ある場合は " [i]" が付く)
		void SetName(const std::string& name);
		// @brief 設定された名前。未設定なら空
		const std::string& GetName() const { return name_; }
		// @brief GPUBufferのリソースバリアを設定する。Flush時に切り替える。
		void TransitionBarrier(ShaderType shaderType, BufferType bufferType);
		// @brief GPUBufferのリソースバリアを設定する。Flush時に切り替える。
		void TransitionBarrier(D3D12_RESOURCE_STATES nextState);
		// @brief GPUBufferの状態を実際にGPUへ反映させる。
		void Flush(ID3D12GraphicsCommandList* cmdList);

		// @brief GPUBufferの状態を初期化する。Flush時に切り替える。 (UAVの場合は使用できない)
		void ZeroReset();
		// @brief UAVバリアを張るか張らないかを設定する。
		void SetUAVBarrier(bool enable = true) { isUAVBarrier_ = enable; }

		// @brief GPUBufferのGPUディスクリプタハンドルを取得する。
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(BufferType type) const;
		// @brief GPUBufferのCPUディスクリプタハンドルを取得する。
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(BufferType type) const;

		// @brief Viewの種類を取得する
		uint8_t GetBufferType() const { return bufferType_; }

		// @brief 現在のフレームで使用しているリソースの、記録上の状態を取得する
		D3D12_RESOURCE_STATES GetCurrentState() const {
			if (currentState_.empty()) return D3D12_RESOURCE_STATE_COMMON;
			return currentState_[currentIndex_ % currentState_.size()];
		}

		ID3D12Resource* GetResource() const {
			if (resources_.empty()) return nullptr;
			return resources_[currentIndex_ % resources_.size()].res.Get();
		}

		// @brief CopyBufferの呼び出し記録(同一フレームでの使用後の書き換え検出用)
		struct CopyRecord {
			std::source_location location;
			bool afterFlush = false;	// そのフレームで既にFlush(使用)された後の書き換えか
		};
		// @brief 最後に「使用後の書き換え」を検出したフレームのCopyBuffer記録。検出していなければ空。Releaseでは常に空
		const std::vector<CopyRecord>& GetMultiWriteRecords() const { return multiWriteRecords_; }
		// @brief 最後に「使用後の書き換え」を検出したフレーム
		uint32_t GetMultiWriteFrame() const { return multiWriteFrame_; }
		// @brief 現在のフレーム番号(FrameCounterが毎フレーム進める)
		static uint32_t GetCurrentFrame() { return currentIndex_; }

		//全体のサイズ
		size_t GetSizeInBytes() const { return sizeInBytes_; }
		//1要素のサイズ
		size_t GetStrideInBytes() const { return strideInBytes_; }
		uint32_t GetNum() const { return num_; }

	protected:

		// @brief 派生クラス(TextureDataなど)用。リソースは作成しないので、派生側でresources_等を設定すること。
		GPUBuffer() = default;

		friend class FrameCounter;
		static void SetCurrentIndex(uint32_t frame) { currentIndex_ = frame; }

		static inline DXDevice* device_ = nullptr;
		
		Logger logger_ = GetLogger("Engine");

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

		bool isUAVBarrier_ = true;

		std::string name_;

		// 同一フレームでの使用後の書き換え検出用(Releaseでは記録しない)
		std::vector<CopyRecord> copyRecords_;			// 現在のフレームのCopyBuffer記録
		uint32_t copyRecordFrame_ = UINT32_MAX;			// copyRecords_のフレーム
		uint32_t lastFlushFrame_ = UINT32_MAX;			// 最後に値をGPUへ書き込んだ(Flushした)フレーム
		std::vector<CopyRecord> multiWriteRecords_;	// 最後に検出したときの記録
		uint32_t multiWriteFrame_ = UINT32_MAX;

		// @brief 使用後の書き換えを検出したときに記録・警告する
		void ReportMultiWrite();

		static inline uint32_t currentIndex_ = 0;
	};

}
