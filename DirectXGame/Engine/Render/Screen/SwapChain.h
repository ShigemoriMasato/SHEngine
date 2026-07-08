#pragma once
#include "Display.h"
#include "WindowsAPI.h"
#include <Render/Command/DirectCommandContext.h>

namespace SHEngine::Screen {

	class SwapChain : public IDisplay {
	public:

		void Initialize(TextureManager* textureManager, DirectCommandContext* directContext, uint32_t clearColor, std::unique_ptr<WindowsAPI> window);

		/// @brief ViewPortを設定する
		void SetViewport(DCC* dcc, Vector2 min = { 0, 0 }, Vector2 size = { 0, 0 }) override;

		/// @brief 画面をClearColorで塗りつぶす
		void Clear(DCC* dcc) override;

		/// @brief 描画後の処理（Present）
		void Present();

		/// @brief RenderTargetのBarrierを張る
		void ToRenderTarget(DCC* dcc) override;

		/// @brief 描画後の処理
		/// @param cmdObject コマンドオブジェト
		void ToPresent(DCC* dcc) override;

		/// @brief バリアをピクセルシェーダーで使用できるようにする
		/// @param cmdObject コマンドオブジェクト
		void ToTexture(DCC* dcc) override;

		/// @brief テクスチャ情報を取得する
		TextureData* GetTextureData() const override { return displays_[currentBufferIndex_]->GetTextureData(); }

		/// @brief 深度テクスチャ情報を取得する
		TextureData* GetDepthTexture() const override { return displays_[currentBufferIndex_]->GetDepthTexture(); }

		/// @brief RTVのフォーマットを取得する
		DXGI_FORMAT GetRTVFormat() override { return DXGI_FORMAT_R8G8B8A8_UNORM;  }

		// @brief RTVの数を取得する
		int GetRTVNum() { return 1; };

		// @brief RTVのCPUHandleを取得する。
		D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle() override { return displays_[currentBufferIndex_]->GetRTVHandle(); }

		// @brief DSVのCPUHandleを取得する。
		D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVHandle() override { return displays_[currentBufferIndex_]->GetDSVHandle(); }

		// @brief ImGuiの描画
		void DrawImGui() {}

		Display* GetCurrentDisplay() { return displays_[swapChain_->GetCurrentBackBufferIndex()].get(); }

		WindowsAPI* GetWindowsAPI() const { return window_.get(); }

	private:

		Logger logger_;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
		std::vector<std::unique_ptr<Display>> displays_{};

		int currentBufferIndex_ = 0;	//現在のバッファのインデックス

		std::unique_ptr<WindowsAPI> window_ = nullptr;
	};

}
