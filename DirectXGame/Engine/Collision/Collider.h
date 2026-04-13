#pragma once
#include <variant>
#include "Data/CollisionInfo.h"
#include <Tool/Logger/Logger.h>

class ColliderManager;

/**
 * @enum CollTag
 * @brief コリジョンタグ(ビットフラグで複数指定可能)
 * 
 * 衝突判定を行う対象を分類するためのタグ。
 * ビット演算により複数のタグを組み合わせて使用できる。
 */
enum class CollTag : uint32_t {
	None,              ///< タグなし
	Player = 1 << 0,   ///< プレイヤー
	Enemy = 1 << 1,    ///< 敵
	Item = 1 << 2,     ///< アイテム
	Slot = 1 << 3,     ///< バックパックの秋スロット
	Chip = 1 << 4,     ///< Pieceのチップ
	Cursor = 1 << 5,   ///< カーソル
	Attack = 1 << 6,   ///< 攻撃オブジェクト
};

/**
 * @brief CollTagのビットOR演算子
 * @return ビットORの結果
 */
uint32_t operator|(CollTag a, CollTag b);

/**
 * @brief uint32_tとCollTagのビットOR演算子
 * @return ビットORの結果
 */
uint32_t operator|(uint32_t a, CollTag b);

/**
 * @brief uint32_tとCollTagのビットAND演算子
 * @return ビットANDの結果
 */
uint32_t operator&(uint32_t a, CollTag b);

/**
 * @brief CollTagとuint32_tのビットAND演算子
 * @return ビットANDの結果
 */
uint32_t operator&(CollTag a, uint32_t b);

/**
 * @brief CollTagのビットAND演算子
 * @return ビットANDの結果
 */
uint32_t operator&(CollTag a, CollTag b);

/**
 * @brief CollTagのビットNOT演算子
 * @return ビットNOTの結果
 */
uint32_t operator~(CollTag a);

/**
 * @struct CollConfig
 * @brief コライダーの設定情報
 * 
 * 衝突判定に必要な情報(自身のタグ、対象タグ、形状、有効状態)をまとめた構造体。
 */
struct CollConfig {
	CollTag ownTag = CollTag::None;                 ///< 自身のタグ
	uint32_t targetTag = 0;                          ///< 衝突対象のタグ(ビットマスク)
	std::variant<Circle*, Quad*, DirCircle*> colliderInfo;      ///< コライダーの形状情報(円または矩形)
	bool isActive = true;                           ///< 有効/無効フラグ
};

class Collider {
public:

	Collider() = default;
	virtual ~Collider();

	/**
	 * @brief ColliderManagerの設定(静的メソッド)
	 * 
	 * 全てのColliderが使用するColliderManagerを設定する。
	 * 
	 * @param manager ColliderManagerのポインタ
	 */
	static void SetColliderManager(ColliderManager* manager) { colliderManager_ = manager; }

	/**
	 * @brief 衝突時のコールバック関数(純粋仮想関数)
	 * 
	 * 他のColliderと衝突した際に呼び出される。
	 * 派生クラスで具体的な衝突処理を実装する。
	 * 
	 * @param other 衝突相手のCollider
	 */
	virtual void OnCollision(Collider* other) {};

	virtual void NonCollision(Collider* other) {};

	// 自分のタグ
	virtual CollTag GetOwnTag() { return ownTag_; }

	/// @brief コライダーの有効状態を取得
	int GetID() const { return id_; }

protected:

	/**
	 * @brief コライダーの初期化
	 * 
	 * ColliderManagerに自身を登録する。
	 */
	void Initialize();

	/**
	 * @brief コライダーの設定
	 * 
	 * @param config コライダーの設定情報
	 */
	void SetColliderConfig(const CollConfig& config);

	/**
	 * @brief コライダーの有効/無効を設定
	 * 
	 * @param isActive 有効にする場合true
	 */
	void SetActive(bool isActive = true) { isActive_ = isActive; };

private:

	/// @brief 全てのColliderが使用するColliderManager(静的メンバ)
	static ColliderManager* colliderManager_;
	/// @brief ログ出力用のLogger(静的メンバ)
	static Logger logger_;
	
private:

	friend class ColliderManager;

	/// @brief コライダーの形状情報(円または矩形)
	std::variant<Circle*, Quad*, DirCircle*> colliderInfo_;
	/// @brief 自身のタグ
	CollTag ownTag_ = CollTag::None;
	/// @brief 衝突対象のタグ(ビットマスク)
	uint32_t targetTag_ = 0;
	/// @brief コライダーの一意なID
	int id_ = -1;

	/// @brief 有効/無効フラグ
	bool isActive_ = true;
};
