#pragma once
#include "Collider.h"
#include <vector>

/**
 * @class ColliderManager
 * @brief 当たり判定全体を管理するマネージャークラス
 * 
 * 登録された全てのCollider間で衝突判定を行い、衝突したペアに対して
 * コールバックを呼び出す責務を持つ。
 */
class ColliderManager {
public:

	/**
	 * @brief 全てのコライダー間の衝突判定を実施
	 * 
	 * 登録されている全てのコライダーを対象に衝突判定を行う。
	 */
	void CollisionCheckAll();

	/**
	 * @brief 特定のコライダーに対する衝突判定
	 * 
	 * 指定されたコライダーと他の全てのコライダーとの間で衝突判定を行う。
	 * 
	 * @param collider 判定対象のコライダー
	 */
	void CollisionCheck(Collider* collider);

	/**
	 * @brief コライダーを登録
	 * 
	 * @param collider 登録するコライダー
	 */
	void AddCollider(Collider* collider);

	/**
	 * @brief コライダーの登録を解除
	 * 
	 * @param collider 解除するコライダー
	 */
	void DeleteCollider(Collider* collider);

private:

	/// @brief 登録されている全てのコライダーのリスト
	std::vector<Collider*> colliders_;

	int nextID_ = 0;

};
