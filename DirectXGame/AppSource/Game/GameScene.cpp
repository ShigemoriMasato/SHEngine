#include "GameScene.h"
#include <Tool/Binary/BinaryManager.h>
#include <Game/Common/ConfigFile.h>
#include <Game/Common/GameMath.h>
#include <algorithm>

GameScene::~GameScene() {
	SaveGameConfigs();
}

void GameScene::Initialize() {
	// ゲーム(カメラを含む)を先に初期化し、パーティクルの描画にもゲームカメラを使う
	InitializeGame();

	particleSystem_.Initialize(directContext_);
	particleSystem_.SetCamera(cameraController_.GetCamera());

	battleField_ = std::make_unique<BattleField>(&particleSystem_, engine_, &imguiManager_, computeContext_);
	battleField_->Initialize();

	postEffectContext_.Initialize(1280, 720);
	bloomEffect_.Initialize(*engine_);
	bloomEffect_.SetMaskTexture(commonData_->display->GetTextureAllData()[1]->GetHandle());
}

std::unique_ptr<IScene> GameScene::Update() {
	// (1) 入力更新と操作デバイス判定
	commonData_->keyManager->Update();
	auto key = commonData_->keyManager->GetKeyStates();

	const float deltaTime = std::clamp(engine_->GetDeltaTime(), 0.0f, kMaxDeltaTime);

	// (2)～(11) ゲーム進行。ゲームオーバー後は更新・生成・攻撃を止める
	if (gameState_ == GameState::Playing) {
		UpdateGame(deltaTime);
	}

	battleField_->Update(deltaTime);

	particleSystem_.Update(computeContext_);
	battleField_->Draw(directContext_);

	auto fieldTextureDrawed = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fieldTextureDrawed);

	battleField_->ApplyTexture(computeContext_);

	auto endApply = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(endApply);

	if (key[Key::One]) {
		battleField_->Explosion(battleField_->GetTestConfig());
	}
	if (key[Key::Two]) {
		battleField_->Initialize();
	}

	// 描画スナップショットをRendererへ渡す(ゲーム状態はRendererから変更しない)
	UpdateRenderers(deltaTime);

	return std::unique_ptr<IScene>();
}

void GameScene::Draw() {
	DefaultPreDraw();
	auto display = commonData_->display.get();

	particleSystem_.Draw(directContext_);

	auto particleDraw = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(particleDraw);

	directContext_->SetRenderTarget(display, false, true);

	//================
	// その他描画処理
	//================

	// ゲーム画面用(Target0)とBloom Mask用(Target1)の両方へ同じ色を出力する
	DrawGame();

	//================
	// その他描画処理
	//================

#ifdef USE_IMGUI
	ImGui::Begin("Bloom");
	bloomEffect_.DrawImGui();
	ImGui::End();
#endif

	DrawGameImGui();

	imguiManager_.DrawImGui();

	display->ToTexture(directContext_);

	postEffectContext_.Begin(display);
	bloomEffect_.Apply(&postEffectContext_);
	postEffectContext_.End();

	DefaultPostDraw();
}

void GameScene::InitializeGame() {
	ConfigFile::Load(FieldConfig::kFileName, fieldConfig_);

	// 参照される側から順に生成し、依存を注入する
	collisionAdapter_ = std::make_unique<CollisionAdapter>();
	bulletManager_ = std::make_unique<BulletManager>(collisionAdapter_.get(), &fieldConfig_);

	gunManager_ = std::make_unique<GunManager>(bulletManager_.get());
	gunManager_->Initialize();

	playerManager_ = std::make_unique<PlayerManager>(collisionAdapter_.get(), &fieldConfig_);
	playerManager_->Initialize(gunManager_->GetGun(GunType::Simple));

	enemyManager_ = std::make_unique<EnemyManager>(collisionAdapter_.get(), &fieldConfig_);
	enemyManager_->Initialize();

	damageResolver_ = std::make_unique<DamageResolver>(playerManager_.get(), enemyManager_.get(), bulletManager_.get());

	const auto size = commonData_->display->GetTextureData()->GetSize();
	const Vector2 screenSize = { static_cast<float>(size.first), static_cast<float>(size.second) };
	cameraController_.Initialize(screenSize, playerManager_->GetWorldPosition());
	aimController_.Initialize();

	// 仮置き: プレイヤー・境界マーカー用のモデルが未作成のため、テスト用Sphereで代用している(必要アセット.md参照)。
	// モデルはModelManager所有のため解放・複製しない
	const ModelData* sphere = modelManager_->GetModelData(SHEngine::TestModel::Sphere);
	playerRenderer_.Initialize(sphere);
	enemyRenderer_.Initialize(modelManager_);
	bulletRenderer_.Initialize(modelManager_);
	fieldRenderer_.Initialize(sphere);
	fieldRenderer_.Update(fieldConfig_);

	gameState_ = GameState::Playing;
}

void GameScene::UpdateGame(float deltaTime) {
	const PlayerInput playerInput = MakePlayerInput();

	// (2) Playerの入力・移動
	playerManager_->Update(deltaTime, playerInput);

	// (3) カメラ追従
	cameraController_.Update(deltaTime, playerManager_->GetWorldPosition());

	// (4) 更新後のカメラ行列から照準を作る(カメラ行列はAimControllerにだけ渡す)
	aimController_.Update(playerManager_->GetWorldPosition(), cameraController_.GetViewMatrix(),
		cameraController_.GetViewProjectionMatrix(), MakeAimInput());

	// (5) 敵の更新・生成
	EnemyTarget target;
	target.position = playerManager_->GetPosition();
	enemyManager_->Update(deltaTime, target);

	// (6) 銃のクールダウン
	gunManager_->Update(deltaTime);

	// (7) 射撃(Key::Fireが有効な間、毎フレームGun::Triggerを呼ぶ)
	playerManager_->Shot(playerInput, aimController_.GetDirection());

	// (8) 弾丸の更新
	bulletManager_->Update(deltaTime);

	// (9) 衝突検出 → ダメージ解決(走査中はイベントを集めるだけで、走査後にまとめて適用する)
	collisionEvents_.clear();
	collisionAdapter_->Detect(collisionEvents_);
	damageResolver_->Resolve(collisionEvents_);

	// (10) 死亡した敵・弾丸のColliderを解除してから削除
	enemyManager_->RemoveDead();
	bulletManager_->RemoveDead();

	// (11) ゲーム状態の遷移
	if (playerManager_->IsDead()) {
		gameState_ = GameState::GameOver;
	}
}

void GameScene::UpdateRenderers(float deltaTime) {
	// ゲームオーバー中は転がり等の見た目も止める
	const float renderDeltaTime = gameState_ == GameState::Playing ? deltaTime : 0.0f;

	playerRenderer_.Update(playerManager_->GetDrawState(), renderDeltaTime);

	enemyManager_->CollectDrawStates(enemyDrawStates_);
	enemyRenderer_.Update(enemyDrawStates_);

	bulletManager_->CollectDrawStates(bulletDrawStates_);
	bulletRenderer_.Update(bulletDrawStates_);
}

void GameScene::DrawGame() {
	const Camera* camera = cameraController_.GetCamera();

	fieldRenderer_.Draw(directContext_, camera);
	enemyRenderer_.Draw(directContext_, camera);
	bulletRenderer_.Draw(directContext_, camera);
	playerRenderer_.Draw(directContext_, camera);
}

void GameScene::DrawGameImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("GameStatus");
	ImGui::Text("State : %s", gameState_ == GameState::Playing ? "Playing" : "GameOver");
	ImGui::Text("HP    : %d / %d", playerManager_->GetHp(), playerManager_->GetConfig().maxHp);
	ImGui::Text("Enemy : %d", static_cast<int>(enemyManager_->GetEnemyCount()));
	ImGui::Text("Bullet: %d", static_cast<int>(bulletManager_->GetBulletCount()));
	ImGui::Text("Device: %s", commonData_->keyManager->GetActiveDevice() == InputDevice::Controller ? "Controller" : "Keyboard/Mouse");
	ImGui::End();
#endif
}

void GameScene::SaveGameConfigs() const {
	// 調整値だけを保存する(実行中状態は保存しない)。初回はファイルが既定値で作られる
	ConfigFile::Save(FieldConfig::kFileName, fieldConfig_);
	cameraController_.SaveConfig();
	aimController_.SaveConfig();
	if (playerManager_) {
		playerManager_->SaveConfig();
	}
	if (enemyManager_) {
		enemyManager_->SaveConfig();
	}
	if (gunManager_) {
		gunManager_->SaveConfig();
	}
}

PlayerInput GameScene::MakePlayerInput() const {
	const KeyManager* keyManager = commonData_->keyManager.get();
	auto key = keyManager->GetKeyStates();

	PlayerInput input;
	input.fire = key[Key::Fire];

	// スティックはデッドゾーン適用後のアナログ量をそのまま使う(一律に長さ1へ正規化しない)
	const Vector2 stick = keyManager->GetLeftStick();
	if (stick.Length() > 0.0f) {
		input.move = stick;
	} else {
		// キーボード(十字キー含む)は縦横を合成し、斜めが速くならないよう長さ1へ正規化する
		Vector2 digital = {
			(key[Key::Right] ? 1.0f : 0.0f) - (key[Key::Left] ? 1.0f : 0.0f),
			(key[Key::Up] ? 1.0f : 0.0f) - (key[Key::Down] ? 1.0f : 0.0f)
		};
		if (!GameMath::TryNormalize(digital, input.move)) {
			input.move = { 0.0f, 0.0f };
		}
	}

	return input;
}

AimInput GameScene::MakeAimInput() const {
	const KeyManager* keyManager = commonData_->keyManager.get();
	const auto size = commonData_->display->GetTextureData()->GetSize();

	AimInput input;
	input.device = keyManager->GetActiveDevice();
	input.cursorPosition = keyManager->GetCursorPos();
	input.rightStick = keyManager->GetRightStick();
	input.screenSize = { static_cast<float>(size.first), static_cast<float>(size.second) };
	return input;
}
