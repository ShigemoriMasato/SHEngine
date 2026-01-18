#pragma once
#include"Utility/Vector.h"
#include"Render/DrawDataManager.h"
#include <Render/RenderObject.h>

enum class OreType {
	Gold,  // 金鉱石

	MaxCount // 数
};

class OreItem {
public:
	virtual ~OreItem() = default;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw(Window* window, const Matrix4x4& vpMatrix) = 0;

	// タイプ
	virtual OreType GetType() = 0;

	// 位置
	virtual Vector3 GetPos() = 0;

	// サイズ
	virtual Vector3 GetSize() = 0;
};