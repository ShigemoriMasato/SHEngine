#pragma once
#include "CommandObject.h"

namespace SHEngine::Command {

	class Queue {
	public:

		Queue(DXDevice* device, Type type);
		~Queue();

		//実行する
		WaitFence Execute(std::vector<Object*> cmdObjs);

		//最後に送ったFenceの値を出力
		uint64_t GetLastSendFence();

		//そのフェンスが完了しているか確認
		bool CheckFinishedJob(uint64_t fence);

		// そのフェンスが完了するまで待つ
		void WaitFenceInCPU(const WaitFence& fence);

		// そのフェンスが完了するまで待つ(別のキューのフェンスも可、GPU側で待機)
		void WaitFenceInGPU(const WaitFence& waitFence);

		//完全にGPUの処理が終わるのを待つ
		void StopGPU();

		//コマンドキューの生ポインタの取得
		ID3D12CommandQueue* GetQueue() { return commandQueue_.Get(); }

	private:

		//コマンドキュー
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

		/// @brief 現在のフェンス値
		UINT64 fenceValue_ = 0;
		/// @brief フェンスオブジェクト
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
		/// @brief フェンスイベントハンドル
		HANDLE fenceEvent_ = nullptr;
	};

}
