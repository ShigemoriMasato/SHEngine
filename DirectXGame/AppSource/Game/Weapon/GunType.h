#pragma once
#include <cstdint>

/// @brief 銃の種類ID。新しい銃はここへ追加し、GunManager::CreateGun で生成処理を追加する
enum class GunType : uint32_t {
	Simple,

	Count
};
