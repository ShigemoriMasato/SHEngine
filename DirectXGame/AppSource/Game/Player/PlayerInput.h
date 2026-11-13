#pragma once
#include <Utility/Vector.h>

/// @brief KeyManagerの結果をゲーム用に変換した入力。GameSceneが作り、Playerが解釈する
struct PlayerInput {
	// 移動入力(XZ, 長さ0～1)。キーボードの斜め入力は正規化済み、スティックはデッドゾーン適用後のアナログ量
	Vector2 move = { 0.0f, 0.0f };
	// Key::Fire(Hold)が有効か
	bool fire = false;
};
