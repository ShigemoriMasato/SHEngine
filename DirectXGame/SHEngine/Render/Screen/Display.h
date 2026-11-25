#pragma once
#include <Core/DXDevice.h>
#include <Assets/Texture/TextureManager.h>
#include "IDisplay.h"
#include <imgui/imgui.h>
#include <memory>
#include <Render/Command/DirectCommandContext.h>

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
		void Initialize(int width, int height, std::string windowName = "");

		void CreateDepthTexture(TextureManager* textureManager);
		void AddRenderTarget(TextureManager* textureManager, ID3D12Resource* resource, uint32_t clearColor);
		void AddRenderTarget(TextureManager* textureManager, uint32_t clearColor, Format format = Format::R8G8B8A8_UNORM, bool unordered = false);

		/// @brief ViewPortを設定する
		/// @param cmdObject コマンドオブジェクト
		/// @param min ViewPortの左上座標
		/// @param size ViewPortのサイズ(初期値の場合TextureSizeが使われる)
		void SetViewport(DCC* dcc, Vector2 min = {0, 0}, Vector2 size = {0, 0}) override;

		void Clear(DCC* dcc) override;
		void ToRenderTarget(DCC* dcc) override;
		void ToPresent(DCC* dcc) override;
		void ToTexture(DCC* dcc) override;
		void ToNonPixel(DCC* dcc) override;
		void ToUnordered(SHEngine::ICommandContext* dcc, bool depthToo = false) override;

		/// @brief 先頭のテクスチャだけ取得する。
		TextureData* GetTextureData() const override { return renderTextures_.front(); }
		/// @brief 全てのテクスチャを取得する。
		std::vector<TextureData*> GetTextureAllData() const { return { renderTextures_.begin(), renderTextures_.end() }; }
		/// @brief 全てのRenderTextureを取得する。
		const std::vector<RenderTexture*>& GetRenderTextures() const { return renderTextures_; }
		TextureData* GetDepthTexture() const override { return depthTexture_; }

		D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle() override;
		D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVHandle() override;

		DXGI_FORMAT GetRTVFormat() override { return rtvFormat_; }
		uint32_t GetRenderTargetNum() override { return static_cast<uint32_t>(renderTextures_.size()); }

	public:	// imgui関係

		// @brief Displayの描画
		void DrawImGui() override;

		// @brief D&Dに突貫工事で対応したDisplayの描画。将来keyを配列にして、返り値もわかりやすくする。多分。
		const ImGuiPayload* DrawImGuiWithDD(std::string key);

		// @brief ImGui上でのカーソル位置を取得する
		// @param cursorPos ウィンドウ基準のカーソル位置
		// @return ImGui上でのカーソル位置 | ImGuiが使われていない時はcursorPosをそのまま返す。
		Vector2 GetCursorPos(Vector2 cursorPos) override;

		// @brief ImGui上でカーソルがDisplayの上にあるかどうか
		// @return カーソルがDisplayの上にあるときtrue、そうでないときfalse | ImGuiが使われていないときは常にtrueを返す。
		bool IsHovering();
		bool IsForcus();

		Vector2 GetPos() override { return imguiPos_; }
		Vector2 GetSize() override { return { imguiWidth_, imguiHeight_ }; }

	private:

		// @brief RenderTextureを追加し、RTVハンドルの配列を更新する
		void AddRenderTexture(RenderTexture* renderTexture);

		// @brief 状態はRenderTexture/DepthTexture(GPUBuffer)側で管理している
		void TransitionBarrier(SHEngine::ICommandContext* dcc, D3D12_RESOURCE_STATES after);
		void TransitionDepthBarrier(SHEngine::ICommandContext* dcc, D3D12_RESOURCE_STATES after);

		static inline Logger logger_ = GetLogger("Engine");

		std::vector<RenderTexture*> renderTextures_ = {};
		DepthTexture* depthTexture_ = nullptr;

		// 関数で渡す用
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandlePtr_{};
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandlePtr_ = { 0 };

		DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		std::string windowName_;

		int width_ = -1;
		int height_ = -1;

		float imguiWidth_ = 0;
		float imguiHeight_ = 0;

		Vector2 imguiPos_ = { 0.0f, 0.0f };

		bool isOffScreen_ = true;
		bool unordered_ = false;
		bool isForcus_ = false;
	};

}
