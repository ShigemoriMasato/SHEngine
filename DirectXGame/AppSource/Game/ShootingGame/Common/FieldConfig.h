#pragma once
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief 円形フィールドの設定。中心・半径・床高さを一元管理する。
/// 反映タイミング: 次プレイから(GameScene::Initialize で読み込み)
struct FieldConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "FieldConfig.bin";

	// フィールド中心(XZ, ワールド単位)。既存の BattleField の床(パーティクル)が原点中心に敷かれているため原点
	Vector2 center = { 0.0f, 0.0f };
	// フィールド半径(ワールド単位, 1～1000)
	float radius = 20.0f;
	// 床のY座標(ワールド単位)。Actorは「床高さ + 自身の半径」の高さに描画する
	float floorHeight = 0.0f;

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void Validate();
};

/// @brief 円形フィールドの境界ルール。PlayerとEnemyで共通に使う。
/// 境界では外向き加速度を0にし、移動後に越境していたら円周上へ戻して外向き速度を0にする(跳ね返さない)。
namespace FieldRule {

	/// @brief 物体の中心が移動できる半径(フィールド半径 - 物体半径)
	float GetMovableRadius(const FieldConfig& field, float bodyRadius);

	/// @brief 境界上にいる場合、加速度から外向き成分を取り除く
	Vector2 RemoveOutwardAcceleration(const FieldConfig& field, const Vector2& position, const Vector2& acceleration, float bodyRadius);

	/// @brief 越境していたら位置を円周上へ戻し、外向きの速度成分を0にする。戻した場合true
	bool Constrain(const FieldConfig& field, Vector2& position, Vector2& velocity, float bodyRadius);

	/// @brief 中心がフィールド半径 + margin より外にあるか
	bool IsOutside(const FieldConfig& field, const Vector2& position, float margin);
}
