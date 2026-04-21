#pragma once
#include "SingleDisplay.h"
#include "WindowsAPI.h"

namespace SHEngine::Screen {

	class SwapChain {
	public:

		void Initialize(DXDevice* device, TextureManager* textureManager, Command::Manager* cmdManager, WindowsAPI* window, uint32_t clearColor);

		void PreDraw(Command::Object* cmdObject, bool isClear = true);
		void ToPresent(Command::Object* cmdObject);
		void ToTexture(Command::Object* cmdObject);
		void Present();

		SingleDisplay* GetCurrentDisplay() { return displays_[swapChain_->GetCurrentBackBufferIndex()].get(); }

	private:

		Logger logger_;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
		std::vector<std::unique_ptr<SingleDisplay>> displays_{};

		int currentBufferIndex_ = 0;	//現在のバッファのインデックス

		static constexpr int bufferCount_ = 3;	//バッファ数
	};

}
