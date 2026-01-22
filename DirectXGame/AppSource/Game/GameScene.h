#pragma once
#include <Scene/IScene.h>
#include <Camera/DebugCamera.h>
#include <Common/DebugParam/ParamManager.h>
#include <Render/PostEffect.h>
#include <Collision/ColliderManager.h>
#include <Game/MiniMap/MiniMap.h>

#include"Unit/UnitManager.h"
#include"Unit/MapChipField.h"
#include"Unit/MapChipRender.h"
#include"Item/OreItemManager.h"
#include"GameCamera/CameraController.h"

class GameScene : public IScene {
public:

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private://エンジン的に必要な奴ら

	DualDisplay* display_ = nullptr;
	GameWindow* gameWindow_ = nullptr;

private:

	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;
	Camera camera_;

	// 当たり判定管理クラス
	std::unique_ptr<ColliderManager> colliderManager_;

	int param1;
	float param2;
	Vector3 param3;
	Vector4 param4;
	std::string param5;

	std::unique_ptr<ParamManager> paramManager_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;
	PostEffectConfig postEffectConfig_{};
	Blur blur_{};

	// ゲームカメラの管理
	std::unique_ptr<CameraController> cameraController_;

	// マップデータの中間管理職
	std::unique_ptr<MapChipField> mapChipField_;

	// マップの描画
	std::unique_ptr<MapChipRender> mapChipRenderer_;

	// 鉱石の管理
	std::unique_ptr<OreItemManager> oreItemManager_;

	// ユニット管理
	std::unique_ptr<UnitManager> unitManager_;

	//MiniMap
	std::unique_ptr<MiniMap> miniMap_ = nullptr;
};
