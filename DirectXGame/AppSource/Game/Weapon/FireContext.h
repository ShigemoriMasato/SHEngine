#pragma once
#include <Utility/Vector.h>

/// @brief 発射時の位置・方向を渡す値型。フレーム内でPlayer → Gun → BulletManagerへ受け渡し、保持しない。
/// Player→Gun では origin = プレイヤー中心、Gun→BulletManager では origin = 銃口のワールド位置になる。
struct FireContext {
	Vector2 origin = { 0.0f, 0.0f };		// 発射元のワールド位置(XZ)
	Vector2 direction = { 0.0f, 1.0f };		// 照準方向(XZ, 単位ベクトル)
};
