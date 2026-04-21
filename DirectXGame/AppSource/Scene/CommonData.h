#pragma once
#include <Render/Screen/WindowsAPI.h>
#include <Render/Screen/SwapChain.h>
#include <Render/Screen/MultiDisplay.h>
#include <Common/KeyConfig/KeyManager.h>
#include <Common/MainDisplay.h>

using WindowSet = std::pair<std::unique_ptr<SHEngine::Screen::WindowsAPI>, std::unique_ptr<SHEngine::Screen::SwapChain>>;

struct CommonData {
	WindowSet mainWindow;	// メインウィンドウとスワップチェーンのセット
	std::unique_ptr<MainDisplay> display = nullptr;	// マルチディスプレイの共通データ
	std::unique_ptr<SHEngine::Command::Object> cmdObject = nullptr;	// コマンドオブジェクトの共通データ

	std::unique_ptr<KeyManager> keyManager = nullptr;	// キーマネージャーの共通データ
	int postEffectDrawDataIndex = -1;
};
