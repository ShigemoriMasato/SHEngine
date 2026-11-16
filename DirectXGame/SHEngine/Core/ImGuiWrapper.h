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

		/// @brief メインウィンドウの外に出した ImGui ウィンドウを更新・描画する。
		/// NewFrame の先頭で自動的に呼ばれる(呼ばれていなければ)。
		/// メインのコマンドリストを実行した後に呼ぶこと。1フレームに何度呼んでもよい
		void RenderPlatformWindows();

		void EndFrame();

		void Finalize();

	private:

		Logger logger_;

		ImFont* font_;

		float width_;
		float height_;

		DXDevice* device_;
		std::vector<SRVHandle> srvHandles_;

		/// @brief このフレームで Render したか(外のウィンドウを描画するかの判定)
		bool isRendered_ = false;
		/// @brief NewFrame 済みで、まだ外のウィンドウを更新していないか
		bool isFrameStarted_ = false;

		constexpr static int bufferNum_ = 3;
	};
}

namespace ImGuizmo {

	void OriginalSetRect(SHEngine::Screen::IDisplay* display);

}
