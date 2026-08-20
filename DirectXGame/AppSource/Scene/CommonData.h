#pragma once
#include <Render/Screen/WindowsAPI.h>
#include <Render/Screen/SwapChain.h>
#include <Render/Screen/Display.h>
#include <Common/KeyConfig/KeyManager.h>
#include <Common/MainDisplay.h>

struct CommonData {
	std::unique_ptr<SHEngine::Screen::SwapChain> window;	// メインウィンドウとスワップチェーンのセット
	std::unique_ptr<SHEngine::Screen::Display> display = nullptr;	// メインディスプレイ
	std::unique_ptr<SHEngine::Screen::Display> subDisplay = nullptr;	// サブディスプレイ

	std::unique_ptr<KeyManager> keyManager = nullptr;	// キーマネージャーの共通データ
	int postEffectDrawDataIndex = -1;

	std::string stageName_ = "Stage1";	// 現在のステージ名
};
