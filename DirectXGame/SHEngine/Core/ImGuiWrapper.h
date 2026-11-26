#pragma once
#include <Core/DXDevice.h>
#include <Core/Command/CommandObject.h>
#include <Render/Screen/WindowsAPI.h>
#include <imgui/imgui.h>
#include <memory>
#include <Render/Command/DirectCommandContext.h>

#include <imgui/ImGuizmo.h>
#include <Camera/Camera.h>
#include <Render/Screen/Display.h>

struct ImGui_ImplDX12_InitInfo;

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
		/// @brief ImGuiのテクスチャ(フォントアトラスなど)用のSRV。バックエンドの要求に応じて確保・解放する
		/// @note ImGui 1.92はフォントアトラスの拡張時に新旧のテクスチャが同時に存在するため、1個では足りない
		std::vector<std::unique_ptr<SRVHandle>> srvHandles_;

		static void AllocateSrvDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu);
		static void FreeSrvDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu);

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
