#pragma once
#include <Render/Command/DirectCommandContext.h>
#include <Assets/Texture/TextureData.h>

namespace SHEngine::Screen {

	class IDisplay {
	public:

		virtual ~IDisplay() = default;

		static void SetDevice(DXDevice* device) { device_ = device; }

		/// @brief ViewPortを設定する
		virtual void SetViewport(DCC* dcc, Vector2 min = { 0, 0 }, Vector2 size = { 0, 0 }) = 0;

		/// @brief 画面をClearColorで塗りつぶす
		virtual void Clear(DCC* dcc) = 0;

		/// @brief RenderTargetのBarrierを張る
		virtual void ToRenderTarget(DCC* dcc) = 0;

		/// @brief 描画後の処理（Presentや、OffScreen用テクスチャへのコピーなど）
		/// @param cmdObject コマンドオブジェト
		virtual void ToPresent(DCC* dcc) = 0;

		/// @brief バリアをピクセルシェーダーで使用できるようにする
		/// @param cmdObject コマンドオブジェクト
		virtual void ToTexture(DCC* dcc) = 0;

		/// @brief Vertex,Computeで使用できる状態にする
		virtual void ToNonPixel(DCC* dcc) = 0;

		/// @brief UAVで使用できる状態にする
		virtual void ToUnordered(SHEngine::ICommandContext* dcc, bool depthToo = false) {};

		/// @brief テクスチャ情報を取得する
		virtual TextureData* GetTextureData() const = 0;

		/// @brief 深度テクスチャ情報を取得する
		virtual TextureData* GetDepthTexture() const = 0;

		/// @brief RTVのフォーマットを取得する
		virtual DXGI_FORMAT GetRTVFormat() = 0;

		// @brief RTVの数を取得する
		virtual int GetRTVNum() { return 1; };

		// @brief RTVのCPUHandleを取得する。
		virtual D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle() = 0;

		// @brief RTVの数を取得する
		virtual uint32_t GetRenderTargetNum() { return 1; }

		// @brief DSVのCPUHandleを取得する。
		virtual D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVHandle() = 0;

		// @brief ImGuiの描画
		virtual void DrawImGui() {}

		// @brief 表示位置。ウィンドウに添わせるのが基本なので0,0を返す。ImGuiを使用しているときはoverrideされる。
		virtual Vector2 GetPos() { return { 0.0f, 0.0f }; }

		// @brief 表示サイズ。ウィンドウに添わせるのが基本なのでTextureSizeを返す。ImGuiを使用しているときはoverrideされる。
		virtual Vector2 GetSize() { auto size = GetTextureData()->GetSize(); return Vector2(float(size.first), float(size.second)); }

	protected:

		void SetViewportInPrivate(DCC* dcc, Vector2 min, Vector2 size);

		static inline DXDevice* device_ = nullptr;

	};

}
