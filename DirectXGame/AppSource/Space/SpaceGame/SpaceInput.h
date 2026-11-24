#pragma once
#include <Utility/Vector.h>
#include <Common/KeyConfig/InputDevice.h>

/// @brief SpaceGameの1フレーム分の入力。KeyManager・Inputの結果をゲーム用に変換したもの
struct SpaceInput {
	// 水平移動(カメラ基準)。x:右が正 y:前が正、長さは0～1
	Vector2 move = { 0.0f, 0.0f };
	// 上下移動。+1で上昇、-1で下降
	float vertical = 0.0f;

	// マウスの移動量(ピクセル)。マウス視点操作が無効なフレームは0
	Vector2 lookMouse = { 0.0f, 0.0f };
	// 右スティック(デッドゾーン適用後)。x:右が正 y:上が正、長さは0～1
	Vector2 lookStick = { 0.0f, 0.0f };

	// 現在操作しているデバイス
	InputDevice device = InputDevice::KeyboardMouse;
};
