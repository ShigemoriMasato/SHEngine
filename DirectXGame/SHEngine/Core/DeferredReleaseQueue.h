#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "View/SRVManager.h"

namespace SHEngine {

	// @brief GPUが使用中かもしれないリソースとディスクリプタ(SRV/UAV)を、数フレーム待ってから解放するキュー。
	// @note DXDeviceが所有する。GPUBufferのデストラクタから自動で使われるので、呼び出し側は意識しなくてよい。
	//       ディスクリプタのスロットも待ってから空けるので、GPU実行中のコマンドが別のリソースを読んでしまうこともない。
	class DeferredReleaseQueue {
	public:

		// @param waitFrames 解放するまでに待つフレーム数
		explicit DeferredReleaseQueue(uint32_t waitFrames);
		// @brief 残っているものはすべて解放する(GPUが停止している前提)
		~DeferredReleaseQueue();

		DeferredReleaseQueue(const DeferredReleaseQueue&) = delete;
		DeferredReleaseQueue& operator=(const DeferredReleaseQueue&) = delete;

		// @brief 現在のキュー。DXDeviceの破棄後(エンジン終了後)はnullptr
		static DeferredReleaseQueue* Get() { return instance_; }

		// @brief 解放待ちに追加する
		void Push(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>&& resources,
			std::vector<std::unique_ptr<SRVHandle>>&& descriptors);

		// @brief 1フレーム進め、待ち終わったものを解放する。毎フレーム1回呼ぶこと
		void Update();

		// @brief すべて即時解放する。GPUの処理がすべて終わっていることを確認してから呼ぶこと
		void ReleaseAll();

		// @brief 解放待ちの数
		size_t GetPendingCount() const { return pending_.size(); }

	private:

		struct Pending {
			uint32_t remainFrames = 0;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resources;
			std::vector<std::unique_ptr<SRVHandle>> descriptors;
		};

		std::vector<Pending> pending_;
		uint32_t waitFrames_ = 0;

		static inline DeferredReleaseQueue* instance_ = nullptr;
	};

}
