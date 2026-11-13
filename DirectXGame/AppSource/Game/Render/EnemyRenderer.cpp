#include "EnemyRenderer.h"
#include <Assets/Model/ModelManager.h>
#include <Game/Common/GameMath.h>
#include <Game/Enemy/EnemyManager.h>

void EnemyRenderer::Initialize(SHEngine::ModelManager* modelManager) {
	for (size_t i = 0; i < renderers_.size(); ++i) {
		const ModelData* model = SelectModel(static_cast<EnemyType>(i), modelManager);
		renderers_[i].renderer.Initialize(model, EnemyManager::kMaxEnemyCount);
		renderers_[i].instances.reserve(EnemyManager::kMaxEnemyCount);
	}
}

void EnemyRenderer::Update(const std::vector<EnemyDrawState>& states) {
	for (auto& typeRenderer : renderers_) {
		typeRenderer.instances.clear();
	}

	for (const EnemyDrawState& state : states) {
		const size_t index = static_cast<size_t>(state.type);
		if (index >= renderers_.size()) {
			continue;
		}

		TypeRenderer& typeRenderer = renderers_[index];
		const float scale = state.radius / typeRenderer.renderer.GetModelRadius();

		ModelInstance instance;
		instance.world = GameMath::MakeWorldMatrix(scale, GameMath::MakeYawMatrix({ state.forward.x, state.forward.z }), state.position);
		instance.color = state.color;
		typeRenderer.instances.push_back(instance);
	}

	for (auto& typeRenderer : renderers_) {
		typeRenderer.renderer.SetInstances(typeRenderer.instances);
	}
}

void EnemyRenderer::Draw(DCC* dcc, const Camera* camera) {
	for (auto& typeRenderer : renderers_) {
		typeRenderer.renderer.Draw(dcc, camera);
	}
}

const ModelData* EnemyRenderer::SelectModel(EnemyType type, SHEngine::ModelManager* modelManager) {
	switch (type) {
	case EnemyType::Chaser:
	default:
		// 仮置き: ChaserEnemy専用モデルが未作成のため、テスト用Sphereで代用している。
		// 将来は敵の種類ごとに異なる形状のモデル(必要アセット.md参照)をここで割り当てる。
		return modelManager->GetModelData(SHEngine::TestModel::Sphere);
	}
}
