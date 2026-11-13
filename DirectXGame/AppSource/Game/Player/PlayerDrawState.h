#pragma once
#include <Utility/Vector.h>

/// @brief プレイヤーの描画スナップショット。PlayerManagerが作り、PlayerRendererへ値で渡す
struct PlayerDrawState {
	Vector3 position = {};			// 球の中心(ワールド)
	Vector3 velocity = {};			// 転がり表現用の速度(ワールド, Y=0)
	float radius = 0.5f;			// 球の半径(ワールド単位)
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool isInvincible = false;		// 被弾後の無敵時間中か(点滅表現用)
	bool isAlive = true;
};
