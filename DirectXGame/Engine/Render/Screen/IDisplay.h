#pragma once
#include <Core/Command/CommandObject.h>
#include <Assets/Texture/TextureData.h>

namespace SHEngine::Screen {

	class IDisplay {
	public:

		virtual ~IDisplay() = default;

		static void SetDevice(DXDevice* device) { device_ = device; }

		/// @brief 画面をClearColorで塗りつぶす
		virtual void Clear(Command::Object* cmdObject) = 0;

		/// @brief RenderTargetのBarrierを張る
		virtual void ToRenderTarget(Command::Object* cmdObject) = 0;

		/// @brief 描画後の処理（Presentや、OffScreen用テクスチャへのコピーなど）
		/// @param cmdObject コマンドオブジェト
		virtual void ToPresent(Command::Object* cmdObject) = 0;

		/// @brief バリアをピクセルシェーダーで使用できるようにする
		/// @param cmdObject コマンドオブジェクト
		virtual void ToTexture(Command::Object* cmdObject) = 0;

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

	protected:

		static inline DXDevice* device_ = nullptr;

	};

}
