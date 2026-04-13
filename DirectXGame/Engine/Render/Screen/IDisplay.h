#pragma once
#include <Core/Command/CommandManager.h>
#include <Assets/Texture/TextureData.h>

namespace SHEngine::Screen {

	class IDisplay {
	public:

		virtual ~IDisplay() = default;

		static void SetDevice(DXDevice* device) { device_ = device; }

		/// @brief レンダーターゲットの切り替えや、画面のクリア
		/// @param cmdObject コマンドオブジェクト
		/// @param isClear 画面をクリアするかどうか
		virtual void PreDraw(Command::Object* cmdObject, bool isClear) = 0;

		/// @brief 描画後の処理（Presentや、OffScreen用テクスチャへのコピーなど）
		/// @param cmdObject コマンドオブジェト
		virtual void PostDraw(Command::Object* cmdObject) = 0;

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

	protected:

		static inline DXDevice* device_ = nullptr;

	};

}
