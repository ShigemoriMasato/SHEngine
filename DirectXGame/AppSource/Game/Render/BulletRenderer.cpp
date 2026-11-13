#include "BulletRenderer.h"
#include <Assets/Model/ModelManager.h>
#include <Game/Bullet/BulletManager.h>
#include <Game/Common/GameMath.h>

void BulletRenderer::Initialize(SHEngine::ModelManager* modelManager) {
	for (size_t i = 0; i < renderers_.size(); ++i) {
		const ModelData* model = SelectModel(static_cast<BulletType>(i), modelManager);
		renderers_[i].renderer.Initialize(model, static_cast<uint32_t>(BulletManager::kMaxBulletCount));
		renderers_[i].instances.reserve(BulletManager::kMaxBulletCount);
	}
}

void BulletRenderer::Update(const std::vector<BulletDrawState>& states) {
	for (auto& typeRenderer : renderers_) {
		typeRenderer.instances.clear();
	}

	for (const BulletDrawState& state : states) {
		const size_t index = static_cast<size_t>(state.type);
		if (index >= renderers_.size()) {
			continue;
		}

		TypeRenderer& typeRenderer = renderers_[index];
		const float scale = state.visualRadius / typeRenderer.renderer.GetModelRadius();

		ModelInstance instance;
		instance.world = GameMath::MakeWorldMatrix(scale, GameMath::MakeYawMatrix({ state.direction.x, state.direction.z }), state.position);
		instance.color = state.color;
		typeRenderer.instances.push_back(instance);
	}

	for (auto& typeRenderer : renderers_) {
		typeRenderer.renderer.SetInstances(typeRenderer.instances);
	}
}

void BulletRenderer::Draw(DCC* dcc, const Camera* camera) {
	for (auto& typeRenderer : renderers_) {
		typeRenderer.renderer.Draw(dcc, camera);
	}
}

const ModelData* BulletRenderer::SelectModel(BulletType type, SHEngine::ModelManager* modelManager) {
	switch (type) {
	case BulletType::Simple:
	default:
		// 仮置き: 弾丸用モデルが未作成のため、テスト用Sphereを縮小して代用している(必要アセット.md参照)
		return modelManager->GetModelData(SHEngine::TestModel::Sphere);
	}
}
