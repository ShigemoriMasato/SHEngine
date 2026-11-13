#pragma once
#include <cstdint>

/// @brief 最後に有効な操作が行われた入力デバイス
enum class InputDevice : uint8_t {
	KeyboardMouse,	// キーボード／マウス
	Controller,		// XBoxコントローラー
};
