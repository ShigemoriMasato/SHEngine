#pragma once
#include <cstdint>

/// @brief 弾丸の種類ID。新しい弾丸はここへ追加し、BulletFactory で生成処理を追加する
enum class BulletType : uint32_t {
	Simple,

	Count
};
