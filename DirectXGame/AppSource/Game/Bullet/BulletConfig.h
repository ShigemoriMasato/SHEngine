#pragma once
#include <Utility/Vector.h>

class BinaryManager;

/// @brief 弾丸の設定(発射位置・方向などのコンテキストとは分離)。
/// 銃の設定(SimpleGunConfig)の一部として保存され、発射時に値でBulletManagerへ渡される。
/// 反映タイミング: 次に発射される弾丸から
struct BulletConfig {
	float speed = 26.0f;			// 初速(ワールド単位/秒, 0.1～500)
	float lifetime = 1.2f;			// 寿命(秒, 0.01～30)
	float radius = 0.15f;			// 当たり判定半径(ワールド単位, 0.01～10)
	float visualRadius = 0.15f;		// 描画上の大きさ(半径, ワールド単位, 0.01～10)
	float height = 0.5f;			// 床からの描画高さ(ワールド単位, 0～100)
	float damage = 1.0f;			// 命中時に敵へ与えるダメージ(0～10000)
	Vector4 color = { 1.0f, 0.85f, 0.3f, 1.0f };	// 描画色(RGBA 0～1)
	bool destroyOnHit = true;		// true: 命中で消滅 / false: 貫通(寿命かフィールド外で消滅)

	void Save(BinaryManager& manager) const;
	void Load(BinaryManager& manager);
	void Validate();
};
