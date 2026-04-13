#pragma once
#include "IDisplay.h"
#include <Assets/Texture/TextureManager.h>

namespace SHEngine::Screen {
	/**
	 * @class MultiDisplay
	 * @brief 複数のディスプレイを管理するクラス
	 *
	 * TripleBufferingに対応したウィンドウを管理し、描画の切り替えやリソースの共有などを行う。
	 */
	class MultiDisplay : public IDisplay {
	public:

		void Initialize(int width, int height, uint32_t clearColor, TextureManager* textureManager);

		void PreDraw(Command::Object* cmdObject, bool isClear = true) override;
		void PostDraw(Command::Object* cmdObject) override;
		void ToTexture(Command::Object* cmdObject) override;

		TextureData* GetTextureData() const override {
			return displays_[currentDisplayIndex_]->GetTextureData();
		}
		TextureData* GetDepthTexture() const override {
			return displays_[currentDisplayIndex_]->GetDepthTexture();
		}

		DXGI_FORMAT GetRTVFormat() override { return DXGI_FORMAT_R8G8B8A8_UNORM; }

	private:

		std::vector<std::unique_ptr<IDisplay>> displays_;	///< 管理するディスプレイのリスト

		int currentDisplayIndex_ = 0;	///< 現在のディスプレイのインデックス

	};

}
