#pragma once
#include <Render/RenderObject.h>
#include"Utility/DataStructures.h"
#include"Game/MapData/Data/MapData.h"
#include<vector>

class MapChipRender {
public:

	// 1つのパーティクルがもつデータ
	struct TransformData {
		Matrix4x4 worldMatrix;
		Transform transform;
		Vector4 color;
		uint32_t textureHandle;
	};

	struct alignas(16) ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
		uint32_t textureHandle;
		float padding[3];
	};

	// 初期化
	void Initialize(DrawData wallDrawData, std::vector<std::vector<TileType>> map);

	// 描画
	void Draw(Window* window, const Matrix4x4& vpMatrix);

private:

	// 各要素のトランスフォーム
	std::vector<TransformData> transformDatas_;

	//　オブジェクトデータ
	std::unique_ptr<RenderObject> renderObject_;


	std::vector<ParticleForGPU> instancingData_;

	int vsDataIndex_ = -1;
	Matrix4x4 worldMatrix_;

	// 実際に描画する数
	uint32_t currentNum_ = 0;
};