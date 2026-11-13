#pragma once
#include "InstancedModelRenderer.h"
#include <Game/Common/FieldConfig.h>
#include <vector>

/// @brief 円形フィールドの境界の描画。境界が見えないと移動範囲が分からないため、
/// 境界用モデルができるまでは小さな球を円周上に並べて仮表示する
class FieldRenderer {
public:

	/// @param model ModelManager所有のモデル(非所有)
	void Initialize(const ModelData* model);

	/// @brief フィールド設定から境界マーカーの配置を作る(設定が変わった時だけ呼べばよい)
	void Update(const FieldConfig& field);

	void Draw(DCC* dcc, const Camera* camera);

private:

	InstancedModelRenderer renderer_;
	std::vector<ModelInstance> instances_;

	static constexpr uint32_t kMarkerCount = 96;		// 円周上のマーカー数
	static constexpr float kMarkerRadius = 0.12f;		// マーカーの半径(ワールド単位)
};
