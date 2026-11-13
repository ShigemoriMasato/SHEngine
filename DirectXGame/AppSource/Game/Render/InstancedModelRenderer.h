#pragma once
#include <Render/Renderer.h>
#include <Assets/Model/ModelData.h>
#include <memory>
#include <vector>

class Camera;

/// @brief 1インスタンス分のGPUデータ(HLSLの Game/Actor/ActorModel.VS.hlsl の InstanceData と同じレイアウト)
struct ModelInstance {
	Matrix4x4 world = Matrix4x4::Identity();
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// x: 仮モデル用の模様の強さ(0で単色, 1で最大)。yzw: 未使用
	Vector4 param = { 0.0f, 0.0f, 0.0f, 0.0f };
};
static_assert(sizeof(ModelInstance) == 96, "ModelInstance must match the HLSL InstanceData layout");

/// @brief 同じモデルをインスタンシングで描画する共通描画部品。
/// モデルはModelManager所有の参照を使い、インスタンスごとに複製しない。
/// 出力: SV_Target0 にゲーム画面の色、SV_Target1(Bloom Mask)に同じ色を書く。
class InstancedModelRenderer {
public:

	/// @param model ModelManager所有のモデル(非所有, 解放・複製しない)
	/// @param maxInstanceCount 同時に描画できる最大インスタンス数
	void Initialize(const ModelData* model, uint32_t maxInstanceCount);

	/// @brief インスタンスデータを転送する。最大数を超えた分は描画しない
	void SetInstances(const std::vector<ModelInstance>& instances);

	/// @brief 描画APIを呼ぶ。描画先は呼び出し側で設定しておく
	void Draw(DCC* dcc, const Camera* camera);

	/// @brief モデル原点から最も遠い頂点までの距離(モデル空間)。スケール計算に使う。
	/// 仕様書はテスト用Sphereを半径0.5と想定しているが、実データ(Assets/.EngineResource/Model/Sphere)は半径1.0のため、
	/// 決め打ちせず頂点から求める。頂点が無い場合は0.5
	float GetModelRadius() const { return modelRadius_; }

private:

	SHEngine::BufferContainer container_;
	SHEngine::GPUBuffer* instanceBuffer_ = nullptr;
	std::vector<std::unique_ptr<SHEngine::Renderer>> renderers_;

	uint32_t maxInstanceCount_ = 0;
	uint32_t instanceCount_ = 0;
	float modelRadius_ = 0.5f;
};
