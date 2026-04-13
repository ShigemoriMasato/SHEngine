#include "Collider.h"
#include "ColliderManager.h"

ColliderManager* Collider::colliderManager_ = nullptr;
Logger Collider::logger_ = getLogger("Collider", LoggerFlag::UseDebugString);

uint32_t operator|(CollTag a, CollTag b) {
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}

uint32_t operator|(uint32_t a, CollTag b) {
    return a | static_cast<uint32_t>(b);
}

uint32_t operator&(uint32_t a, CollTag b) {
    return a & static_cast<uint32_t>(b);
}

uint32_t operator&(CollTag a, uint32_t b) {
	return static_cast<uint32_t>(a) & b;
}

uint32_t operator&(CollTag a, CollTag b) {
	return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
}

uint32_t operator~(CollTag a) {
    return ~static_cast<uint32_t>(a);
}

Collider::~Collider() {
    if (colliderManager_) {
        colliderManager_->DeleteCollider(this);
	}
}

void Collider::Initialize() {
	isActive_ = true;

    if (!colliderManager_) {
		logger_->error("ColliderManager is not set. Please set it before initializing Collider.");
        return;
    }

	colliderManager_->AddCollider(this);
}

void Collider::SetColliderConfig(const CollConfig& config) {
    ownTag_ = config.ownTag;
    targetTag_ = config.targetTag;
	colliderInfo_ = config.colliderInfo;
	isActive_ = config.isActive;
}
