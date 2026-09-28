#pragma once
#include <Utility/Vector.h>

struct ICollider {};

struct Circle : public ICollider {
	Vector2 center;
	float radius;
};

struct Quad : public ICollider {
	Vector2 topLeft;
	Vector2 bottomRight;
};

struct DirCircle : public ICollider {
	Vector2 center;				//中心座標
	float radius;				//半径
	Vector2 direction;			//方向ベクトル
	float radian;				//扇の内角
};
