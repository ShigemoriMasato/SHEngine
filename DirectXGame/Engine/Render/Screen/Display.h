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
		/// @param rtNum RenderTargetの数
		/// @param windowName ウィンドウ名（ImGuiやlogで表示される）
		void Initialize(TextureManager* textureManager, int width, int height, uint32_t clearColor, uint32_t rtNum = 1, std::string windowName = "");

		/// @brief スワップチェーン用初期化関数
		/// @param textureManager テクスチャマネージャーへのポインタ
		/// @param resource スワップチェーンのリソース
		void Initialize(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor, std::string windowName = "");

		void Clear(Command::Object* cmdObject) override;
		void ToRenderTarget(Command::Object* cmdObject) override;
		void ToPresent(Command::Object* cmdObject) override;
		void ToTexture(Command::Object* cmdObject) override;

		/// @brief 先頭のテクスチャだけ取得する。
		TextureData* GetTextureData() const override { return textureData_.front(); }
		/// @brief 全てのテクスチャを取得する。
		std::vector<TextureData*> GetTextureAllData() const { return textureData_; }
		TextureData* GetDepthTexture() const override { return depthTextureData_; }

		D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle() override;
		D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVHandle() override;

		DXGI_FORMAT GetRTVFormat() override { return rtvFormat_; }
		uint32_t GetRenderTargetNum() override { return static_cast<uint32_t>(textureData_.size()); }

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

		void PrivateInitialize(SHEngine::TextureManager* textureManager, uint32_t rtNum, std::string windowName = "");

		void TransitionBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after);
		void TransitionDepthBarrier(Command::Object* cmdObject, D3D12_RESOURCE_STATES after);

		std::vector<TextureData*> textureData_ = {};
		TextureData* depthTextureData_ = nullptr;

		std::vector<RTVHandle> rtvHandle_{};
		DSVHandle dsvHandle_{};

		// 関数で渡す用
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandlePtr_{};
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandlePtr_;

		DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		std::vector<D3D12_RESOURCE_STATES> currentBarrier_;
		D3D12_RESOURCE_STATES currentDepthBarrier_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		std::string windowName_;

		int width_ = 0;
		int height_ = 0;

		bool isOffScreen_ = false;
	};

}
