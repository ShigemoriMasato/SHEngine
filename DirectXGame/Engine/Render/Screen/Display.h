#pragma once
#include <Core/DXDevice.h>
#include <Assets/Texture/TextureManager.h>
#include "IDisplay.h"

namespace SHEngine::Screen {

	class Display final : public IDisplay {
	public:

		/// @brief OffScreen用初期化関数
		/// @param textureManager テクスチャマネージャーへのポインタ
		/// @param width 幅
		/// @param height 高さ
		/// @param clearColor クリアカラー
		void Initialize(TextureManager* textureManager, int width, int height, uint32_t clearColor, std::string windowName = "");

		/// @brief スワップチェーン用初期化関数
		/// @param textureManager テクスチャマネージャーへのポインタ
		/// @param resource スワップチェーンのリソース
		void Initialize(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor, std::string windowName = "");

		void Clear(Command::Object* cmdObject) override;
		void ToRenderTarget(Command::Object* cmdObject) override;
		void ToPresent(Command::Object* cmdObject) override;
		void ToTexture(Command::Object* cmdObject) override;

		TextureData* GetTextureData() const override { return textureData_; }
		TextureData* GetDepthTexture() const override { return depthTextureData_; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() override { return rtvHandle_.GetCPU(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() override { return dsvHandle_.GetCPU(); }

		DXGI_FORMAT GetRTVFormat() override { return rtvFormat_; }

	public:	// imgui関係

		// @brief Displayの描画
		void DrawImGui() override;

		// @brief ImGui上でのカーソル位置を取得する
		// @param windowCursor ウィンドウ基準のカーソル位置
		// @return ImGui上でのカーソル位置 | ImGuiが使われていない時はwindowCursorをそのまま返す。
		Vector2 GetCursorPos(Vector2 windowCursor);

		// @brief ImGui上でカーソルがDisplayの上にあるかどうか
		// @return カーソルがDisplayの上にあるときtrue、そうでないときfalse | ImGuiが使われていないときは常にtrueを返す。
		bool IsHovering();

	private:

		void PrivateInitialize(SHEngine::TextureManager* textureManager, std::string windowName = "");

		void TransitionBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after);
		void TransitionDepthBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after);

		TextureData* textureData_ = nullptr;
		TextureData* depthTextureData_ = nullptr;

		RTVHandle rtvHandle_{};
		DSVHandle dsvHandle_{};

		DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		D3D12_RESOURCE_STATES currentBarrier_ = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES currentDepthBarrier_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		std::string windowName_;

		int width_ = 0;
		int height_ = 0;

		bool isOffScreen_ = false;
	};

}
