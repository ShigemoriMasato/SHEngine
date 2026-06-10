#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandManager.h>
#include <Render/Screen/WindowsAPI.h>
#include <imgui/imgui.h>
#include <Render/Command/DirectCommandContext.h>

namespace SHEngine {
	class ImGuiWrapper {
	public:

		void Initialize(DXDevice* device, DirectCommandContext* directContext, Screen::WindowsAPI* window);

		void NewFrame();

		void Render(CmdObj* cmdObj);

		void EndFrame();

		void Finalize();

	private:

		Logger logger_;

		ImFont* font_;

		float width_;
		float height_;

		DXDevice* device_;
		std::vector<SRVHandle> srvHandles_;

		constexpr static int bufferNum_ = 3;
	};
}
