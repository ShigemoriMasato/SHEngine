#pragma once
#include <Render/Screen/Display.h>
#include "Base/IPostEffect.h"

namespace SHEngine {

	class PostEffectLauncher {
	public:

		void Boot(SHEngine::Screen::IDisplay* display);
		void Launch(IPostEffect* effect);
		void End();

	private:

		SHEngine::Screen::IDisplay* currentDisplay_ = nullptr;
		std::unique_ptr<SHEngine::Screen::Display> intermediate_ = nullptr;

		// ポストエフェクトをかけたい色が乗っている方
		SHEngine::Screen::IDisplay* source_ = nullptr;
		// 次の出力先
		SHEngine::Screen::IDisplay* destination_ = nullptr;

	};

}