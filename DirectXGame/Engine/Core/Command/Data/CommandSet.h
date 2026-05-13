#pragma once
#include <Core/DXDevice.h>

namespace SHEngine::Command {

	enum class Type {
		Direct,
		Texture = Direct,
		Compute
	};

	class Queue;

	class DXList {
	public:

		~DXList();

		/// @brief 初期化
		void Initialize(DXDevice* device, Type type);

		/// @brief コマンドを積めるかどうか
		bool CanExecute() const;
		/// @brief コマンドを積めるようになるまで待機する
		void WaitForCanExecute();
		/// @brief コマンドを実行できる状態にして渡す
		void Execute(Queue* queue, std::vector<ID3D12CommandList*>& cmdLists);

		/// @brief コマンドリストを取得
		ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }

		void ResetCommandList();

	private:

		DXDevice* device_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;

		std::vector<std::pair<Queue*, uint64_t>> executed_;		//実行中のキューとフェンス。実行できるかのチェック用
	};
}
