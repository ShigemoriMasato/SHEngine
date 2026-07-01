#pragma once
#include <Core/DXDevice.h>

namespace SHEngine::Command {

	enum class Type {
		Direct,
		Texture = Direct,
		Compute
	};

	struct WaitFence {
		ID3D12Fence* fence;
		HANDLE fenceEvent;
		UINT64 value = 0;
	};

	class Queue;

	class DXList {
	public:

		~DXList();

		/// @brief 初期化
		void Initialize(DXDevice* device, Type type);

		/// @brief コマンドを実行できる状態にして渡す
		void Execute(std::vector<ID3D12CommandList*>& cmdLists);

		/// @brief フェンスの情報をセットする
		void SetFence(WaitFence fence) { currentFence_ = fence; }

		/// @brief コマンドを積めるかどうか
		bool CanExecute();

		/// @brief GPUの処理がすべて終わるのをCPU側で待機する
		void WaitFenceInCPU();

		/// @brief コマンドリストを取得
		ID3D12GraphicsCommandList6* GetCommandList() { return commandList_.Get(); }

		void ResetCommandList();

	private:

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> commandList_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;

		std::vector<std::pair<Queue*, uint64_t>> executed_;		//実行中のキューとフェンス。実行できるかのチェック用

		ID3D12DescriptorHeap* srvHeap_ = nullptr;	//SRV用のヒープ

		WaitFence currentFence_;	//現在のフェンスの情報
	};
}
