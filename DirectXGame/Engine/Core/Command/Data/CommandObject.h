#pragma once
#include "CommandSet.h"

namespace SHEngine::Command {

	class Manager;

	class Object {
	public:

		Object(DXDevice* device, Manager* manager, Type type, Queue* queue, int listNum);
		~Object();

		/// @brief コマンドを積めるかどうか
		bool CanExecute();

		/// @brief コマンドリストをリセットして、コマンドを積める状態にする(実行できる状態でなかったら実行できるまで待つ)
		void ResetCommandList();

		/// @brief GPUの処理がすべて終わるのを待機する
		void WaitForGPUIdle();

		/// @brief コマンドリストを取得
		ID3D12GraphicsCommandList* GetCommandList() { return commandLists_[dxListIndex_].GetCommandList(); }

		int GetListIndex() const { return dxListIndex_; }

		/// @brief 現在の状態を簡単に文字列であらわす
		std::string Log() const;

	private:

		friend class Queue;

		/// @brief CommandListを実行する
		void Execute(std::vector<ID3D12CommandList*>& cmdLists);

		int dxListIndex_ = 0;

		std::vector<DXList> commandLists_;
		DXDevice* device_ = nullptr;

		//デストラクタで解放する用
		Manager* manager_ = nullptr;
		Type type_;
		Queue* queue_;

		//コマンドリストの状態管理
		enum class State {
			Close,		// コマンドリストがクローズされている状態。コマンドを積めない
			Open,		// コマンドリストがオープンされている状態。コマンドを積める
		} state_;
	};

}
