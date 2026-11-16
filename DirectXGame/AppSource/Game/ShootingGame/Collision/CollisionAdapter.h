#pragma once
#include "CollisionEvent.h"
#include <Collision/ColliderManager.h>
#include <Utility/Vector.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

/// @brief 当たり判定を持つ物体の分類
enum class BodyKind : uint32_t {
	Player,
	Enemy,
	Bullet,
};

/// @brief ゲームと既存の Collider / ColliderManager の境界。
/// Collider の登録・解除・形状データの寿命・コールバックをここに閉じ込め、Player/Enemy/Bullet のルールが
/// ColliderManager の内部構造に依存しないようにする。
/// 既存の ColliderManager は判定時点の位置を比べる離散判定のため、弾丸と敵については前フレーム位置を使った
/// 移動軌跡(相対移動)の判定もここで追加で行う。
/// 寿命: このクラスに Body を登録する各Managerより長く生きること(GameSceneで先に宣言する)。
class CollisionAdapter {
public:

	CollisionAdapter();
	~CollisionAdapter();
	CollisionAdapter(const CollisionAdapter&) = delete;
	CollisionAdapter& operator=(const CollisionAdapter&) = delete;

	/// @brief Body を登録する(XZ平面上の円)。同じ kind/id が登録済みなら半径だけ更新する
	void Register(BodyKind kind, uint32_t id, float radius);
	/// @brief Body の登録を解除し、Collider を破棄する。未登録なら何もしない
	void Unregister(BodyKind kind, uint32_t id);
	/// @brief Body の前フレーム位置・現在位置・半径を更新する
	void UpdateBody(BodyKind kind, uint32_t id, const Vector2& previousPosition, const Vector2& position, float radius);
	/// @brief Body を衝突判定の対象にするか(死亡済みで削除待ちのものを外すために使う)
	void SetEnabled(BodyKind kind, uint32_t id, bool enabled);

	/// @brief (9) 全 Body の衝突を走査し、重複を除いたイベントを out に追加する。
	/// 走査中は管理コンテナやHPを変更せず、イベントの収集だけを行う
	void Detect(std::vector<CollisionEvent>& out);

	size_t GetBodyCount() const { return bodies_.size(); }

private:

	class GameCollider;
	friend class GameCollider;

	static uint64_t MakeKey(BodyKind kind, uint32_t id);
	GameCollider* Find(BodyKind kind, uint32_t id) const;

	/// @brief Collider のコールバックから呼ばれる。ペアを一時キューへ積むだけ
	void QueueHit(const GameCollider& self, Collider* other);
	/// @brief 弾丸と敵の移動軌跡による判定(離散判定のすり抜け対策)
	void DetectSweptHits();

	// 宣言順に注意: bodies_ が先に破棄され、Collider のデストラクタが manager_ から自身を解除する
	ColliderManager manager_;
	std::unordered_map<uint64_t, std::unique_ptr<GameCollider>> bodies_;

	// (自身のキー, 相手のキー) の組。Detect中だけ使う
	std::vector<std::pair<uint64_t, uint64_t>> queuedHits_;
};
