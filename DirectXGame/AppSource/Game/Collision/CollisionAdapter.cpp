#include "CollisionAdapter.h"
#include <Collision/Collider.h>
#include <Game/Common/GameMath.h>
#include <algorithm>

namespace {
	CollTag ToOwnTag(BodyKind kind) {
		switch (kind) {
		case BodyKind::Player:
			return CollTag::Player;
		case BodyKind::Enemy:
			return CollTag::Enemy;
		case BodyKind::Bullet:
			return CollTag::Attack;
		default:
			return CollTag::None;
		}
	}

	// コールバックを受け取る側だけが相手タグを持つ。敵は自分からは判定しない
	uint32_t ToTargetMask(BodyKind kind) {
		switch (kind) {
		case BodyKind::Player:
			return static_cast<uint32_t>(CollTag::Enemy);
		case BodyKind::Bullet:
			return static_cast<uint32_t>(CollTag::Enemy);
		default:
			return 0;
		}
	}

	/// @brief 相対移動を考慮した円同士の移動軌跡判定。
	/// 敵の座標系で弾丸の相対位置を前フレーム→現在で線分移動させ、半径の和の円と交差するか調べる
	bool SweptCircleHit(const Vector2& aPrevious, const Vector2& aCurrent, const Vector2& bPrevious, const Vector2& bCurrent, float radiusSum) {
		const Vector2 start = aPrevious - bPrevious;
		const Vector2 end = aCurrent - bCurrent;
		const float radiusSquared = radiusSum * radiusSum;

		if (GameMath::Dot(start, start) <= radiusSquared || GameMath::Dot(end, end) <= radiusSquared) {
			return true;
		}

		const Vector2 move = end - start;
		const float moveSquared = GameMath::Dot(move, move);
		if (moveSquared <= 1.0e-12f) {
			return false;
		}

		const float t = std::clamp(-GameMath::Dot(start, move) / moveSquared, 0.0f, 1.0f);
		const Vector2 closest = start + move * t;
		return GameMath::Dot(closest, closest) <= radiusSquared;
	}
}

/// @brief 既存の Collider を継承したゲーム用Collider。形状(Circle)を自身で所有し、Colliderより先に破棄されないようにする
class CollisionAdapter::GameCollider : public Collider {
public:

	GameCollider(CollisionAdapter* owner, BodyKind kind, uint32_t id, float radius)
		: owner_(owner), kind_(kind), bodyID_(id) {
		circle_.center = { 0.0f, 0.0f };
		circle_.radius = radius;

		CollConfig config;
		config.ownTag = ToOwnTag(kind);
		config.targetTag = ToTargetMask(kind);
		config.colliderInfo = &circle_;
		config.isActive = true;
		SetColliderConfig(config);

		// ColliderManagerへ登録する
		Collider::Initialize();
	}

	void OnCollision(Collider* other) override {
		if (enabled_) {
			owner_->QueueHit(*this, other);
		}
	}

	void SetBody(const Vector2& previousPosition, const Vector2& position, float radius) {
		previous_ = previousPosition;
		circle_.center = position;
		circle_.radius = radius;
	}

	void SetEnabled(bool enabled) {
		enabled_ = enabled;
		SetActive(enabled);
	}

	void SetRadius(float radius) { circle_.radius = radius; }

	BodyKind GetKind() const { return kind_; }
	uint32_t GetBodyID() const { return bodyID_; }
	bool IsEnabled() const { return enabled_; }
	const Vector2& GetPrevious() const { return previous_; }
	const Vector2& GetCurrent() const { return circle_.center; }
	float GetRadius() const { return circle_.radius; }

private:

	CollisionAdapter* owner_ = nullptr;
	BodyKind kind_;
	uint32_t bodyID_;
	Circle circle_{};
	Vector2 previous_ = {};
	bool enabled_ = true;
};

CollisionAdapter::CollisionAdapter() {
	Collider::SetColliderManager(&manager_);
}

CollisionAdapter::~CollisionAdapter() {
	// Collider のデストラクタが manager_ から自身を解除するため、manager_ より先に全て破棄する
	bodies_.clear();
	Collider::SetColliderManager(nullptr);
}

uint64_t CollisionAdapter::MakeKey(BodyKind kind, uint32_t id) {
	return (static_cast<uint64_t>(kind) << 32) | static_cast<uint64_t>(id);
}

CollisionAdapter::GameCollider* CollisionAdapter::Find(BodyKind kind, uint32_t id) const {
	auto it = bodies_.find(MakeKey(kind, id));
	if (it == bodies_.end()) {
		return nullptr;
	}
	return it->second.get();
}

void CollisionAdapter::Register(BodyKind kind, uint32_t id, float radius) {
	if (GameCollider* body = Find(kind, id)) {
		body->SetRadius(radius);
		return;
	}
	bodies_.emplace(MakeKey(kind, id), std::make_unique<GameCollider>(this, kind, id, radius));
}

void CollisionAdapter::Unregister(BodyKind kind, uint32_t id) {
	bodies_.erase(MakeKey(kind, id));
}

void CollisionAdapter::UpdateBody(BodyKind kind, uint32_t id, const Vector2& previousPosition, const Vector2& position, float radius) {
	if (GameCollider* body = Find(kind, id)) {
		body->SetBody(previousPosition, position, radius);
	}
}

void CollisionAdapter::SetEnabled(BodyKind kind, uint32_t id, bool enabled) {
	if (GameCollider* body = Find(kind, id)) {
		body->SetEnabled(enabled);
	}
}

void CollisionAdapter::QueueHit(const GameCollider& self, Collider* other) {
	// このマネージャーに登録されるのはGameColliderだけだが、念のため型を確認する
	const GameCollider* otherBody = dynamic_cast<const GameCollider*>(other);
	if (!otherBody || !otherBody->IsEnabled()) {
		return;
	}
	queuedHits_.emplace_back(MakeKey(self.GetKind(), self.GetBodyID()), MakeKey(otherBody->GetKind(), otherBody->GetBodyID()));
}

void CollisionAdapter::DetectSweptHits() {
	std::vector<const GameCollider*> bullets;
	std::vector<const GameCollider*> enemies;
	for (const auto& [key, body] : bodies_) {
		if (!body->IsEnabled()) {
			continue;
		}
		if (body->GetKind() == BodyKind::Bullet) {
			bullets.push_back(body.get());
		} else if (body->GetKind() == BodyKind::Enemy) {
			enemies.push_back(body.get());
		}
	}

	for (const GameCollider* bullet : bullets) {
		for (const GameCollider* enemy : enemies) {
			if (SweptCircleHit(bullet->GetPrevious(), bullet->GetCurrent(), enemy->GetPrevious(), enemy->GetCurrent(),
				bullet->GetRadius() + enemy->GetRadius())) {
				queuedHits_.emplace_back(MakeKey(BodyKind::Bullet, bullet->GetBodyID()), MakeKey(BodyKind::Enemy, enemy->GetBodyID()));
			}
		}
	}
}

void CollisionAdapter::Detect(std::vector<CollisionEvent>& out) {
	queuedHits_.clear();

	// 既存の離散判定(コールバックではキューへ積むだけ)
	manager_.CollisionCheckAll();
	// 弾丸の移動軌跡による判定
	DetectSweptHits();

	// 同じ組み合わせは1フレームに1回だけ通知する
	std::sort(queuedHits_.begin(), queuedHits_.end());
	queuedHits_.erase(std::unique(queuedHits_.begin(), queuedHits_.end()), queuedHits_.end());

	for (const auto& [selfKey, otherKey] : queuedHits_) {
		const BodyKind selfKind = static_cast<BodyKind>(selfKey >> 32);
		const BodyKind otherKind = static_cast<BodyKind>(otherKey >> 32);
		const uint32_t selfID = static_cast<uint32_t>(selfKey & 0xffffffffull);
		const uint32_t otherID = static_cast<uint32_t>(otherKey & 0xffffffffull);

		if (otherKind != BodyKind::Enemy) {
			continue;
		}

		CollisionEvent event;
		event.enemyID = otherID;
		if (selfKind == BodyKind::Player) {
			event.type = CollisionEventType::PlayerEnemyContact;
		} else if (selfKind == BodyKind::Bullet) {
			event.type = CollisionEventType::BulletEnemyHit;
			event.sourceID = selfID;
		} else {
			continue;
		}
		out.push_back(event);
	}

	queuedHits_.clear();
}
