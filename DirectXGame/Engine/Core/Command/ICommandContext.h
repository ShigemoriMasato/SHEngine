#pragma once
#include <Core/Command/CommandObject.h>
#include <Core/Command/SHCmdQueue.h>
#include "Measure/MeasureShaderTime.h"

namespace SHEngine {

	class ICommandContext {
	public:

		virtual ~ICommandContext() = default;

		/// @brief コマンドオブジェクトを取得する。Fenceを取得するごとに切り替わるので、都度取得して使用すること。
		CmdObj* GetCurrentCmdObj();

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

		void WaitFenceInGPU(const Command::WaitFence& waitFence) { queue_->WaitFenceInGPU(waitFence); }

		void WaitFenceInCPU(const Command::WaitFence& waitFence) { queue_->WaitFenceInCPU(waitFence); }

	public:// 計測関係

		void BeginTimeStamp(std::string name);

		void EndTimeStamp();

		double GetTimeStampResult(std::string name);

	protected:

		void PrivateInitialize(DXDevice* device, Command::Type type, int initCmdObjNum = 2);

	private:

		std::unique_ptr<Command::Queue> queue_ = nullptr;

		std::vector<std::unique_ptr<CmdObj>> cmdObjects_;

		int currentCmdObjIndex_ = 0;

		std::vector<Command::WaitFence> lastWaitFence_ = {};

		DXDevice* device_ = nullptr;

		Command::Type type_ = Command::Type::Direct;

		std::unique_ptr<MeasureShaderTime> measureShaderTime_ = nullptr;

	};
}
