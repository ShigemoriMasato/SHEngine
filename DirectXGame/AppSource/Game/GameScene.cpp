#include "GameScene.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"FpsCount.h"
#include"GameCamera/DebugMousePos.h"

namespace {
	//Minimap確認用
	bool poseMode = false;

	std::string playerModelName = "Player";
}

void GameScene::Initialize() {
	gameWindow_ = commonData_->mainWindow.get();
	display_ = commonData_->display.get();
	//カメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	//パラメーター管理の初期化
	paramManager_ = std::make_unique<ParamManager>();

	//　当たり判定管理クラスを初期化
	colliderManager_ = std::make_unique<ColliderManager>();
	// 当たり判定管理クラスを登録
	Collider::SetColliderManager(colliderManager_.get());

	auto inst = GameParamEditor::GetInstance();
	inst->SetActiveScene("GameScene");
	inst->CreateGroup("Test", "GameScene");
	inst->AddItem("Test", "param1", param1, 0);
	inst->AddItem("Test", "param2", param2, 1);
	inst->AddItem("Test", "param3", param3, 2);
	inst->AddItem("Test", "param4", param4, 3);
	inst->AddItem("Test", "param5", param5, 4);

	//==================================================
	// カメラシステム
	//==================================================

	// ゲームカメラを設定
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(input_);

	//============================================
	// マップシステム
	//============================================

	// マップデータ解釈機能を初期化
	mapChipField_ = std::make_unique<MapChipField>();
	// デバックで使用するマップデータを構築
	mapChipField_->SetDebugMapData();

	// 壁モデルを取得
	int wallModelID = modelManager_->LoadModel(playerModelName);
	auto wallModel = modelManager_->GetNodeModelData(wallModelID);

	// マップの描画機能を初期化
	mapChipRenderer_ = std::make_unique<MapChipRender>();
	mapChipRenderer_->Initialize(drawDataManager_->GetDrawData(wallModel.drawDataIndex), mapChipField_->GetMapData());

	//================================================================
	// 鉱石システム
	//================================================================

	// 鉱石モデル
	int oreItemModelID = modelManager_->LoadModel(playerModelName);
	auto oreItemModel = modelManager_->GetNodeModelData(oreItemModelID);

	// 鉱石の管理システムを初期化
	oreItemManager_ = std::make_unique<OreItemManager>();
	oreItemManager_->Initialize(drawDataManager_->GetDrawData(oreItemModel.drawDataIndex));

	//============================================================================
	// ユニットシステム
	//============================================================================

	// プレイヤーモデルを取得
	int playerModelID = modelManager_->LoadModel(playerModelName);
	auto playerModel = modelManager_->GetNodeModelData(playerModelID);

	// おれモデルを取得
	int oreModelID = modelManager_->LoadModel(playerModelName);
	auto oreModel = modelManager_->GetNodeModelData(oreModelID);

	// ユニットの管理クラス
	unitManager_ = std::make_unique<UnitManager>();
	unitManager_->Initalize(mapChipField_.get(),
		drawDataManager_->GetDrawData(playerModel.drawDataIndex), drawDataManager_->GetDrawData(oreModel.drawDataIndex),
		commonData_->keyManager.get());

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(textureManager_, drawDataManager_->GetDrawData(commonData_->postEffectDrawDataIndex));

	postEffectConfig_.window = gameWindow_->GetWindow();
	postEffectConfig_.origin = display_;
	postEffectConfig_.jobs_ = (uint32_t) PostEffectJob::Blur | PostEffectJob::Fade | PostEffectJob::Glitch;

	//============================================================================
	// ユニットシステム
	//============================================================================
	miniMap_ = std::make_unique<MiniMap>();
	miniMap_->Initialize((int)mapChipField_->GetMapData()[0].size(), (int)mapChipField_->GetMapData().size(), textureManager_);
}

std::unique_ptr<IScene> GameScene::Update() {

	// Δタイムを取得する
	FpsCount::deltaTime = engine_->GetFPSObserver()->GetDeltatime();

	// 入力処理の更新
	input_->Update();
	commonData_->keyManager->Update();

	debugCamera_->Update();
	camera_ = *static_cast<Camera*>(debugCamera_.get());

	//====================================================
	// カメラの更新庶路
	//====================================================

	// マウスのスクリーン座標を取得する
	POINT cursorPos;
	if (GetCursorPos(&cursorPos)) {
		// スクリーン座標をクライアント座標に変換
		ScreenToClient(gameWindow_->GetWindow()->GetHwnd(), &cursorPos);
		// カーソル位置をワールド座標に変換
		DebugMousePos::screenMousePos = { static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y) };
	}
	// カメラの更新処理
	cameraController_->Update();

	//==============================================================
	// ユニットの選択処理
	//==============================================================

	// マウスの位置に鉱石が存在していればユニットを動かす
	Vector3 oreWorldPos = {};
	if (oreItemManager_->IsSelectOre(cameraController_->GetWorldPos(), oreWorldPos)) {

		// 左クリックを取得
		if ((Input::GetMouseButtonState()[0] & 0x80) && !(Input::GetPreMouseButtonState()[0] & 0x80)) {

			// 選択された鉱石を取得
			OreItem* selectedOreItem = oreItemManager_->GetOreItemForId();

			// 追加出来るかを確認
			int32_t deletaNum = selectedOreItem->IsFullWorker(unitManager_->GetUnitSpawnNum());
			if (deletaNum >= 0) {

				// おれを追加
				unitManager_->RegisterUnit(selectedOreItem->GetPos(), 0);

				// 鉱石側の労働者カウントを増やす
				for (int i = 0; i < unitManager_->GetUnitSpawnNum(); ++i) {
					selectedOreItem->AddWorker();
				}
			} else {
				if (unitManager_->GetUnitSpawnNum() >= deletaNum * -1.0f) {
					// おれを追加
					unitManager_->RegisterUnit(selectedOreItem->GetPos(), deletaNum);

					// 鉱石側の労働者カウントを増やす
					int32_t actualSpawnCount = unitManager_->GetUnitSpawnNum() + deletaNum;
					for (int i = 0; i < actualSpawnCount; ++i) {
						selectedOreItem->AddWorker();
					}
				}	
			}
		}
	}

	//=====================================================
	// 鉱石の更新処理
	//=====================================================

	// 鉱石管理の更新処理
	oreItemManager_->Update();

	//====================================================================
	// ユニットの更新処理
	//====================================================================

	// ユニットの更新処理
	unitManager_->Update();

	//============================================
	// 当たり判定
	//============================================

	// 全ての当たり判定を判定
	colliderManager_->CollisionCheckAll();

	return nullptr;
}

void GameScene::Draw() {
	for (int i = 0; i < 2; ++i) {
		Matrix4x4 vpMatrix{};
		if (i == 0) {
			vpMatrix = miniMap_->PreDraw(gameWindow_->GetWindow())->GetVPMatrix();
		} else {
			display_->PreDraw(gameWindow_->GetCommandObject(), true);
			// カメラ行列
			vpMatrix = cameraController_->GetVpMatrix();
		}

		// マップを描画
		mapChipRenderer_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// 鉱石の描画
		oreItemManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		// ユニットを描画
		unitManager_->Draw(gameWindow_->GetWindow(), vpMatrix);

		display_->PostDraw(gameWindow_->GetCommandObject());

		//PostEffectとか
		postEffect_->Draw(postEffectConfig_);

		if (i == 0) {
			miniMap_->PostDraw(gameWindow_->GetWindow());
		} else {
			gameWindow_->PreDraw();
		}
	}

	//ImGui
	miniMap_->DrawImGui();
	gameWindow_->DrawDisplayWithImGui();
	paramManager_->Draw();

	// カメラのデバック情報
	cameraController_->DebugDraw();

	engine_->ImGuiDraw();
}
