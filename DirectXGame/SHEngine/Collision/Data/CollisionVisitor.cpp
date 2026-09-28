#include "CollisionVisitor.h"
#include <algorithm>

bool CollisionVisitor::operator()(Circle* colliderA, Circle* colliderB) {
	float dist = (colliderA->center - colliderB->center).Length();
	float radiusSum = colliderA->radius + colliderB->radius;

	if (dist <= radiusSum) {
		return true;
	}
	return false;
}

bool CollisionVisitor::operator()(Circle* colliderA, Quad* colliderB) {
	float closestX = std::clamp(colliderA->center.x, colliderB->topLeft.x, colliderB->bottomRight.x);
	float closestY = std::clamp(colliderA->center.y, colliderB->topLeft.y, colliderB->bottomRight.y);
	Vector2 closestPoint{ closestX, closestY };
	float dist = (colliderA->center - closestPoint).Length();
	if (dist <= colliderA->radius) {
		return true;
	}
	return false;
}

bool CollisionVisitor::operator()(Quad* colliderA, Circle* colliderB) {
	return CollisionVisitor::operator()(colliderB, colliderA);
}

bool CollisionVisitor::operator()(Quad* colliderA, Quad* colliderB) {
	if (colliderA->topLeft.x > colliderB->bottomRight.x ||
		colliderA->bottomRight.x < colliderB->topLeft.x ||
		colliderA->topLeft.y > colliderB->bottomRight.y ||
		colliderA->bottomRight.y < colliderB->topLeft.y) {
		return false;
	}
	return true;
}

bool CollisionVisitor::operator()(DirCircle* colliderA, Circle* colliderB) {
	Vector2 toTarget = colliderB->center - colliderA->center;
	float dist = toTarget.Length();
	if (dist > colliderA->radius + colliderB->radius) {
		return false;
	}
	if (dist == 0.0f) {
		return true; // 完全に重なっている場合は当たりとする
	}
	toTarget = toTarget.Normalize();
	Vector2 dir = colliderA->direction.Normalize();
	float dot = toTarget.x * dir.x + toTarget.y * dir.y;
	dot = std::clamp(dot, -1.0f, 1.0f); // acosfの引数は-1から1の範囲でなければならない
	float limit = cosf(colliderA->radian * 0.5f);
	if (dot >= limit) {
		return true;
	}
	return false;
}

bool CollisionVisitor::operator()(Circle* colliderA, DirCircle* colliderB) {
	return CollisionVisitor::operator()(colliderB, colliderA);
}

bool CollisionVisitor::operator()(DirCircle* colliderA, Quad* colliderB) {
	Vector2 closestPoint{
		std::clamp(colliderA->center.x, colliderB->topLeft.x, colliderB->bottomRight.x),
		std::clamp(colliderA->center.y, colliderB->topLeft.y, colliderB->bottomRight.y)
	};
	Vector2 toTarget = closestPoint - colliderA->center;
	float dist = toTarget.Length();

	if (dist > colliderA->radius) {
		return false;
	}

	toTarget = toTarget.Normalize();
	float dot = toTarget.x * colliderA->direction.x + toTarget.y * colliderA->direction.y;
	float angle = acosf(dot);
	if (angle <= colliderA->radian / 2.0f) {
		return true;
	}
	return false;
}

bool CollisionVisitor::operator()(DirCircle* colliderA, DirCircle* colliderB) {
	Vector2 toTarget = colliderB->center - colliderA->center;
	float dist = toTarget.Length();
	if (dist > colliderA->radius + colliderB->radius) {
		return false;
	}
	
	toTarget = toTarget.Normalize();
	float dot = toTarget.x * colliderA->direction.x + toTarget.y * colliderA->direction.y;
	float angle = acosf(dot);
	if (angle <= colliderA->radian / 2.0f) {
		return true;
	}
	return false;
}

bool CollisionVisitor::operator()(Quad* colliderA, DirCircle* colliderB) {
	return CollisionVisitor::operator()(colliderB, colliderA);
}
