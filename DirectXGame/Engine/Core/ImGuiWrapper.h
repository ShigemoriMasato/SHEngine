#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandManager.h>
#include <Render/Screen/WindowsAPI.h>
#include <imgui/imgui.h>

namespace SHEngine {
	class ImGuiWrapper {
	public:

		void Initialize(DXDevice* device, Command::Manager* manager, Screen::WindowsAPI* window, Command::Object* cmdObject);

		void NewFrame();

		void Render();

		void EndFrame();

		void Finalize();

	private:

		Logger logger_;

		ImFont* font_;

		float width_;
		float height_;

		DXDevice* device_;
		std::vector<SRVHandle> srvHandles_;

		Command::Object* cmdObject_;

		constexpr static int bufferNum_ = 3;
	};
}
