#pragma once
#include "CommandSet.h"
#include <Core/FrameCounter.h>

namespace SHEngine::Screen {
	class IDisplay;
}

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

		/// @brief RenderTargetを設定する。
		void SetRenderTarget(Screen::IDisplay* display, bool clear = true);

		/// @brief RenderTargetを取得する
		Screen::IDisplay* GetRenderTarget() const { return renderTarget_; }

		/// @brief コマンドリストを取得
		ID3D12GraphicsCommandList* GetCommandList() { return commandLists_[currentIndex_ % uint32_t(commandLists_.size())].GetCommandList(); }

		/// @brief 現在の状態を簡単に文字列であらわす
		std::string Log() const;

	private:

		friend class Queue;

		/// @brief CommandListを実行する
		void Execute(std::vector<ID3D12CommandList*>& cmdLists);

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

		//描画先の管理
		Screen::IDisplay* renderTarget_ = nullptr;


		friend class SHEngine::FrameCounter;
		static void SetCurrentIndex(uint32_t frame) {
			currentIndex_ = frame;
		}
		static inline uint32_t currentIndex_ = 0;
	};

}
