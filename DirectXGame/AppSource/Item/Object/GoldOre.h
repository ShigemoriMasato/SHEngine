#pragma once
#include"Collision/Collider.h"

#include"OreItem.h"
#include"OreObject.h"

class GoldOre : public OreItem,public Collider {
public:

	void Initialize(DrawData drawData, const Vector3& pos);

	void Update() override;

	void Draw(Window* window, const Matrix4x4& vpMatrix) override;

	void OnCollision(Collider* other) override {
		other;
	}

public:

	// タイプ
	OreType GetType() override { return OreType::Gold; }

	// 位置
	Vector3 GetPos() override { return object_->transform_.position; }

	// サイズ
	Vector3 GetSize() override { return size_; }

private:
	// オブジェクトデータ
	std::unique_ptr<OreObject> object_;

	int32_t hp_ = 0;

	Vector3 size_ = {3.0f,3.0f,3.0f};
};