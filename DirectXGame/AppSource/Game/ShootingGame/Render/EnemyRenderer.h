#pragma once
#include "InstancedModelRenderer.h"
#include <Game/ShootingGame/Enemy/EnemyDrawState.h>
#include <Game/ShootingGame/Enemy/EnemyType.h>
#include <array>
#include <unordered_map>
#include <vector>

namespace SHEngine {
	class ModelManager;
}

/// @brief 敵の描画。EnemyDrawState(値)だけを受け取り、種類ごとに選んだモデルでインスタンシング描画する。
/// プレイヤーと同様に、速度から球が転がる回転を個体ごとに求める(回転は見た目専用の描画側状態として保持する)
class EnemyRenderer {
public:

	/// @param modelManager モデル取得元(非所有)。取得したモデルはModelManager所有のまま参照する
	void Initialize(SHEngine::ModelManager* modelManager);

	/// @brief 描画スナップショットを受け取り、転がり回転を進めてインスタンスデータを更新する
	void Update(const std::vector<EnemyDrawState>& states, float deltaTime);
	void Draw(DCC* dcc, const Camera* camera);

private:

	/// @brief 敵の種類ごとのモデル選択。将来は種類ごとに専用モデルを割り当てる
	static const ModelData* SelectModel(EnemyType type, SHEngine::ModelManager* modelManager);

	struct TypeRenderer {
		InstancedModelRenderer renderer;
		std::vector<ModelInstance> instances;
	};
	std::array<TypeRenderer, static_cast<size_t>(EnemyType::Count)> renderers_;

	// 個体ID → 転がりの累積回転(見た目専用)。描画対象から外れた個体の分は毎フレーム破棄する
	std::unordered_map<uint32_t, Matrix4x4> rollRotations_;
	std::unordered_map<uint32_t, Matrix4x4> nextRollRotations_;
};
