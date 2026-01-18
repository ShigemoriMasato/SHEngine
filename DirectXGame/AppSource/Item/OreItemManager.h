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

	/// <summary>
	/// 選択した位置に鉱石が存在しているかを取得
	/// </summary>
	/// <param name="selectpos">選択する位置</param>
	/// <param name="worldPos">選択した鉱石の位置</param>
	/// <returns></returns>
	bool IsSelectOre(const Vector3 selectpos,Vector3& worldPos);

private:

	// 鉱石の描画データ
	DrawData goldOreDrawData_;

	// 鉱石達
	std::vector<std::unique_ptr<OreItem>> oreItems_;
};