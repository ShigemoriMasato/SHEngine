#include "EnemyRenderer.h"
#include <Assets/Model/ModelManager.h>
#include <Game/ShootingGame/Common/GameMath.h>
#include <Game/ShootingGame/Enemy/EnemyManager.h>

void EnemyRenderer::Initialize(SHEngine::ModelManager* modelManager) {
	for (size_t i = 0; i < renderers_.size(); ++i) {
		const ModelData* model = SelectModel(static_cast<EnemyType>(i), modelManager);
		renderers_[i].renderer.Initialize(model, EnemyManager::kMaxEnemyCount);
		renderers_[i].instances.reserve(EnemyManager::kMaxEnemyCount);
	}
}

void EnemyRenderer::Update(const std::vector<EnemyDrawState>& states, float deltaTime) {
	for (auto& typeRenderer : renderers_) {
		typeRenderer.instances.clear();
	}
	nextRollRotations_.clear();

	for (const EnemyDrawState& state : states) {
		const size_t index = static_cast<size_t>(state.type);
		if (index >= renderers_.size()) {
			continue;
		}

		// 初登場の個体は無回転の姿勢から転がり始める
		Matrix4x4 rollRotation = Matrix4x4::Identity();
		if (auto it = rollRotations_.find(state.id); it != rollRotations_.end()) {
			rollRotation = it->second;
		}

		// 球の転がり: 接地点が滑らない回転 ω = up × v / r を積算する(PlayerRendererと同じ)
		const float speed = state.velocity.Length();
		if (speed > GameMath::kStopSpeed && state.radius > 0.0f) {
			Vector3 axis;
			if (GameMath::TryNormalize(GameMath::Cross({ 0.0f, 1.0f, 0.0f }, state.velocity), axis)) {
				const float angle = speed * deltaTime / state.radius;
				rollRotation = GameMath::Orthonormalize(rollRotation * GameMath::MakeAxisAngleMatrix(axis, angle));
			}
		}
		nextRollRotations_[state.id] = rollRotation;

		TypeRenderer& typeRenderer = renderers_[index];
		const float scale = state.radius / typeRenderer.renderer.GetModelRadius();

		ModelInstance instance;
		instance.world = GameMath::MakeWorldMatrix(scale, rollRotation, state.position);
		typeRenderer.instances.push_back(instance);
	}

	// 今回描画しなかった個体(撃破済みなど)の回転を捨てる
	rollRotations_.swap(nextRollRotations_);

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
		return modelManager->LoadModel("GameObject/Enemy/Chaser");
	}
}
