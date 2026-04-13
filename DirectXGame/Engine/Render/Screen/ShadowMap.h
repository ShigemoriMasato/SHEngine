#pragma once
#include "IDisplay.h"
#include <Assets/Texture/TextureManager.h>

namespace SHEngine::Screen {

	/// @brief シャドウマップ用のスクリーン
	class ShadowMap : public IDisplay {
	public:

		void Initialize(SHEngine::TextureManager* textureManager);

		void PreDraw(Command::Object* cmdObject, bool isClear) override;

		void PostDraw(Command::Object* cmdObject) override;

		void ToTexture(Command::Object* cmdObject) override;

		TextureData* GetTextureData() const override { return {}; }

		DXGI_FORMAT GetRTVFormat() override { return DXGI_FORMAT_UNKNOWN; }

		int GetRTVNum() override { return 0; }

	private:

		void TransitionBarrier(CmdObj* cmdObj, D3D12_RESOURCE_STATES afterState);

		std::unique_ptr<DSVHandle> dsvHandle_ = nullptr;
		TextureData* depthTextureData_ = nullptr;
		D3D12_RESOURCE_STATES beforeState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	};

}
