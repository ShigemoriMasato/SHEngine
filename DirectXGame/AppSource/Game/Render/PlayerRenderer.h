#pragma once
#include "InstancedModelRenderer.h"
#include <Game/Player/PlayerDrawState.h>

/// @brief プレイヤーの描画。PlayerDrawState(値)だけを受け取り、ゲーム状態は変更しない。
/// 速度から球が転がる回転を求める(回転は見た目専用の描画側状態として保持する)。
class PlayerRenderer {
public:

	/// @param model ModelManager所有のモデル(非所有)
	void Initialize(const ModelData* model);

	/// @brief 描画スナップショットを受け取り、転がり回転を進めてインスタンスデータを更新する
	void Update(const PlayerDrawState& state, float deltaTime);

	void Draw(DCC* dcc, const Camera* camera);

private:

	InstancedModelRenderer renderer_;
	std::vector<ModelInstance> instances_;

	// 転がりの累積回転(見た目専用)
	Matrix4x4 rollRotation_ = Matrix4x4::Identity();
	float blinkTimer_ = 0.0f;

	// 無敵中の点滅周期(秒)
	static constexpr float kBlinkInterval = 0.08f;
	// 転がりを見せるための模様の強さ(仮モデル用)
	static constexpr float kPatternStrength = 0.6f;
};
