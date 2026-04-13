#pragma once
#include "CommandObject.h"

namespace SHEngine::Command {

	class Queue {
	public:

		Queue(DXDevice* device, Type type);
		~Queue();

		//CommandObjectの登録
		void RegisterObject(Object* object);

		//実行する
		void Execute(std::vector<Object*> cmdObjs);

		//最後に送ったFenceの値を出力
		uint64_t GetLastSendFence();

		//そのフェンス
		bool CheckFinishedJob(uint64_t fence);

		//作業
		void WaitForFence(uint64_t fence);

		//完全にGPUの処理が終わるのを待つ
		void StopGPU();

		//コマンドキューの生ポインタの取得
		ID3D12CommandQueue* GetQueue() { return commandQueue_.Get(); }

	private:

		//コマンドキュー
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

		std::vector<Object*> objects_;

		/// @brief 現在のフェンス値
		UINT64 fenceValue_ = 0;
		/// @brief フェンスオブジェクト
		Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
		/// @brief フェンスイベントハンドル
		HANDLE fenceEvent_ = nullptr;
	};

}
