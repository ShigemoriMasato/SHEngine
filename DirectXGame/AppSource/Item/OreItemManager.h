#pragma once
#include<vector>

#include"Object/OreItem.h"
#include"Object/GoldOre.h"

class OreItemManager {
public:

	void Initialize(const DrawData& goldOreDrawData);

	void Update();

	void Draw(Window* window, const Matrix4x4& vpMatrix);

public:

	void AddOreItem(OreType type, const Vector3& pos);

private:

	// 鉱石の描画データ
	DrawData goldOreDrawData_;

	// 鉱石達
	std::vector<std::unique_ptr<OreItem>> oreItems_;
};