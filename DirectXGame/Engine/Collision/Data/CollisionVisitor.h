#pragma once
#include "CollisionInfo.h"

class CollisionVisitor {
public:
	bool operator()(Circle* colliderA, Circle* colliderB);
	bool operator()(Circle* colliderA, Quad* colliderB);
	bool operator()(Circle* colliderA, DirCircle* colliderB);
	bool operator()(Quad* colliderA, Circle* colliderB);
	bool operator()(Quad* colliderA, Quad* colliderB);
	bool operator()(Quad* colliderA, DirCircle* colliderB);
	bool operator()(DirCircle* colliderA, Circle* colliderB);
	bool operator()(DirCircle* colliderA, Quad* colliderB);
	bool operator()(DirCircle* colliderA, DirCircle* colliderB);

	bool IsHit() const { return isHit_; }

private:
	bool isHit_ = false;
};
