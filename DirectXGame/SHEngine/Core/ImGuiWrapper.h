#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>
#include <Render/Screen/WindowsAPI.h>
#include <imgui/imgui.h>
#include <Render/Command/DirectCommandContext.h>

#include <imgui/ImGuizmo.h>
#include <Camera/Camera.h>
#include <Render/Screen/Display.h>

namespace SHEngine {
	class ImGuiWrapper {
	public:

		void Initialize(DXDevice* device, DirectCommandContext* directContext, Screen::WindowsAPI* window);

		void NewFrame();

		void Render(ID3D12GraphicsCommandList* cmdList);

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

namespace ImGuizmo {

	void OriginalSetRect(SHEngine::Screen::IDisplay* display);

}
