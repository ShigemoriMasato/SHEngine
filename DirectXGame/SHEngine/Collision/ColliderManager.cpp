#include "ColliderManager.h"
#include <Collision/Data/CollisionVisitor.h>

void ColliderManager::CollisionCheckAll() {
	for (const auto& collider : colliders_) {
		if (collider->isActive_) {
			CollisionCheck(collider);
		}
	}
}

void ColliderManager::CollisionCheck(Collider* collider) {
	for (const auto& other : colliders_) {
		if (!collider->isActive_) {
			continue;
		}

		//互いに当たってしかるべきタグか確認
		if (!(collider->targetTag_ & other->ownTag_) || (collider == other)) {
			continue;
		}

		//当たり判定処理
		bool isHit = std::visit(CollisionVisitor(), collider->colliderInfo_, other->colliderInfo_);
		if (isHit) {
			collider->OnCollision(other);
		} else {
			collider->NonCollision(other);
		}
	}
}

void ColliderManager::AddCollider(Collider* collider) {
	collider->id_ = nextID_++;
	colliders_.emplace_back(collider);
}

void ColliderManager::DeleteCollider(Collider* collider) {
	for (size_t i = 0; i < colliders_.size(); ++i) {
		if (colliders_[i] == collider) {
			colliders_.erase(colliders_.begin() + i--);
		}
	}
}
