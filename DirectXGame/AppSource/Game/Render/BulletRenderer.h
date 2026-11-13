#pragma once
#include "InstancedModelRenderer.h"
#include <Game/Bullet/BulletDrawState.h>
#include <Game/Bullet/BulletType.h>
#include <array>
#include <vector>

namespace SHEngine {
	class ModelManager;
}

/// @brief 弾丸の描画。BulletDrawState(値)だけを受け取り、種類ごとに選んだモデルでインスタンシング描画する
class BulletRenderer {
public:

	/// @param modelManager モデル取得元(非所有)
	void Initialize(SHEngine::ModelManager* modelManager);

	void Update(const std::vector<BulletDrawState>& states);
	void Draw(DCC* dcc, const Camera* camera);

private:

	/// @brief 弾丸の種類ごとのモデル選択
	static const ModelData* SelectModel(BulletType type, SHEngine::ModelManager* modelManager);

	struct TypeRenderer {
		InstancedModelRenderer renderer;
		std::vector<ModelInstance> instances;
	};
	std::array<TypeRenderer, static_cast<size_t>(BulletType::Count)> renderers_;
};
