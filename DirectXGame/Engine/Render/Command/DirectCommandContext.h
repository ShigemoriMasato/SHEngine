#pragma once
#include <Core/Command/Data/CommandObject.h>
#include <Core/Command/Data/SHCmdQueue.h>

namespace SHEngine {

	class DirectCommandContext {
	public:

		void Initialize(DXDevice* device, int initCmdObjNum = 3);

		/// @brief コマンドオブジェクトを取得する。Fenceを取得するごとに切り替わるので、都度取得して使用すること。
		CmdObj* GetCurrentCmdObj() { return cmdObjects_[currentCmdObjIndex_].get(); }

		/// @brief コマンドを積むためのコマンドオブジェクトを準備する
		void BeginFrame();

		/// @brief コマンドを実行して、CmdObjを切り替え。GPUが処理を終えるのを待つためのフェンスを返す
		Command::WaitFence MiddleExecute();

		/// @brief コマンドを実行する
		void EndFrame();

		// @brief コマンドキューの生ポインタの取得
		ID3D12CommandQueue* GetCommandQueue() { return queue_->GetQueue(); }

		// @brief GPUの処理がすべて終わるのを待つ
		void StopGPU() { queue_->StopGPU(); }

	private:

		std::unique_ptr<Command::Queue> queue_ = nullptr;

		std::vector<std::unique_ptr<Command::Object>> cmdObjects_;

		int currentCmdObjIndex_ = 0;

		std::vector<Command::WaitFence> lastWaitFence_ = {};

		DXDevice* device_ = nullptr;

	};
}
