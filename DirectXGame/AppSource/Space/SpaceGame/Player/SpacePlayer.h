#pragma once
#include <Utility/Vector.h>
#include <cstdint>
#include <string>

class BinaryManager;

/// @brief SpacePlayerの調整値。実行中状態(位置・速度)は含めない
struct SpacePlayerConfig {
	static constexpr uint32_t kVersion = 1;
	static inline const std::string kFileName = "SpacePlayerConfig.bin";

	float horizontalSpeed = 10.0f;	// XZ平面の最大速度(ワールド単位/秒, 0～1000)。即時反映
	float verticalSpeed = 8.0f;		// 上昇・下降の最大速度(ワールド単位/秒, 0～1000)。即時反映
	float responsiveness = 8.0f;	// 目標速度への追従の鋭さ(1/秒, 0.1～100)。大きいほどキビキビ動く。即時反映
	float radius = 0.5f;			// 描画半径(ワールド単位, 0.05～100)。即時反映

	void Save(BinaryManager& manager) const;
	bool Load(BinaryManager& manager);
	void DrawImGui();
	void Validate();
};

/// @brief 宇宙空間を移動する白い球。入力(ワールド方向)から速度・位置を更新するだけで、描画は持たない
class SpacePlayer {
public:

	void Initialize();

	/// @param moveWorld XZ平面上の移動方向(ワールド, x = X, y = Z)。長さ0～1
	/// @param vertical 上下入力(-1～1)
	void Update(float deltaTime, const Vector2& moveWorld, float vertical);

	void DrawImGui();
	void SaveConfig() const;

	const Vector3& GetPosition() const { return position_; }
	const Vector3& GetVelocity() const { return velocity_; }
	float GetRadius() const { return config_.radius; }

private:

	SpacePlayerConfig config_{};

	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
};
