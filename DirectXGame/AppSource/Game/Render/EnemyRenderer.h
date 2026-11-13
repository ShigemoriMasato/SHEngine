#pragma once
#include "InstancedModelRenderer.h"
#include <Game/Enemy/EnemyDrawState.h>
#include <Game/Enemy/EnemyType.h>
#include <array>
#include <vector>

namespace SHEngine {
	class ModelManager;
}

/// @brief 敵の描画。EnemyDrawState(値)だけを受け取り、種類ごとに選んだモデルでインスタンシング描画する
class EnemyRenderer {
public:

	/// @param modelManager モデル取得元(非所有)。取得したモデルはModelManager所有のまま参照する
	void Initialize(SHEngine::ModelManager* modelManager);

	void Update(const std::vector<EnemyDrawState>& states);
	void Draw(DCC* dcc, const Camera* camera);

private:

	/// @brief 敵の種類ごとのモデル選択。将来は種類ごとに専用モデルを割り当てる
	static const ModelData* SelectModel(EnemyType type, SHEngine::ModelManager* modelManager);

	struct TypeRenderer {
		InstancedModelRenderer renderer;
		std::vector<ModelInstance> instances;
	};
	std::array<TypeRenderer, static_cast<size_t>(EnemyType::Count)> renderers_;
};
