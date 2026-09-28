#pragma once
#include <Core/Command/ICommandContext.h>

namespace SHEngine {

	class DirectCommandContext : public ICommandContext {
	public:

		void Initialize(DXDevice* device, int initCmdObjNum = 2);

		Command::WaitFence MiddleExecute() override;

		void SetRenderTarget(Screen::IDisplay* display, bool clear = true, bool setViewPort = true);

		Screen::IDisplay* GetRenderTarget() { return renderTarget_; }

	private:

		Screen::IDisplay* renderTarget_ = nullptr;

	};
}

using DCC = SHEngine::DirectCommandContext;
