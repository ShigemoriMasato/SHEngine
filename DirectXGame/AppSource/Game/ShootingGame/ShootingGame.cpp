#include "ShootingGame.h"
#include <Common/KeyConfig/KeyManager.h>
#include <Particle/ParticleSystem.h>
#include <Tool/ImGui/ImGuiWindowManager.h>
#include <Game/ShootingGame/Common/ConfigFile.h>
#include <Game/ShootingGame/Common/GameMath.h>

ShootingGame::ShootingGame(SHEngine::Engine* engine, const KeyManager* keyManager, ParticleSystem* particleSystem, ImGuiWindowManager* imguiManager)
	: engine_(engine)
	, directContext_(engine->GetDirectCommandContext())
	, computeContext_(engine->GetComputeCommandContext())
	, keyManager_(keyManager)
	, particleSystem_(particleSystem)
	, imguiManager_(imguiManager) {
}

ShootingGame::~ShootingGame() {
	SaveConfigs();
}

void ShootingGame::Initialize(const Vector2& screenSize) {
	screenSize_ = screenSize;

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

	cameraController_.Initialize(screenSize_, playerManager_->GetWorldPosition());
	aimController_.Initialize();

	// 仮置き: プレイヤー・境界マーカー用のモデルが未作成のため、テスト用Sphereで代用している(必要アセット.md参照)。
	// モデルはModelManager所有のため解放・複製しない
	SHEngine::ModelManager* modelManager = engine_->GetModelManager();
	const ModelData* sphere = modelManager->GetModelData(SHEngine::TestModel::Sphere);
	playerRenderer_.Initialize(sphere);
	enemyRenderer_.Initialize(modelManager);
	bulletRenderer_.Initialize(modelManager);
	fieldRenderer_.Initialize(sphere);
	fieldRenderer_.Update(fieldConfig_);

	// ImGuiManagerに描画関数を登録する
	imguiManager_->PushGroup("Camera");
	imguiManager_->Add([&]() {
		cameraController_.ConfigImGui();
		}, "Config");
	imguiManager_->Add([&]() {
		cameraController_.DrawImGui();
		}, "Curve");
	imguiManager_->PopGroup();

	imguiManager_->Add([&]() {
		playerManager_->DrawImGui();
		}, "Player");
}

void ShootingGame::Update(float deltaTime) {
	if (isGameOver_) {
		return;
	}

	UpdateGame(deltaTime);
}

void ShootingGame::ApplyField() {
	auto fieldTextureDrawed = directContext_->MiddleExecute();
	computeContext_->WaitFenceInGPU(fieldTextureDrawed);

	auto endApply = computeContext_->MiddleExecute();
	directContext_->WaitFenceInGPU(endApply);
}

void ShootingGame::UpdateGame(float deltaTime) {
	const PlayerInput playerInput = MakePlayerInput();

	//Playerの入力・移動
	playerManager_->Update(deltaTime, playerInput);

	if (playerManager_->IsSpecialActivated()) {
		cameraController_.Boot("Special");
		// todo: 敵を瞬殺するオブジェクトの生成
	}

	// カメラアニメーション
	const float systemDeltaTime = engine_->GetDeltaTime();
	cameraController_.UpdateCurve(systemDeltaTime);
	//カメラ追従
	cameraController_.Update(deltaTime, playerManager_->GetWorldPosition());

	//更新後のカメラ行列から照準を作る(カメラ行列はAimControllerにだけ渡す)
	aimController_.Update(playerManager_->GetWorldPosition(), cameraController_.GetViewMatrix(),
		cameraController_.GetViewProjectionMatrix(), MakeAimInput());

	//敵の更新・生成
	EnemyTarget target;
	target.position = playerManager_->GetPosition();
	enemyManager_->Update(deltaTime, target);

	//銃のクールダウン
	gunManager_->Update(deltaTime);

	//射撃(Key::Fireが有効な間、毎フレームGun::Triggerを呼ぶ)
	playerManager_->Shot(playerInput, aimController_.GetDirection());

	//弾丸の更新
	bulletManager_->Update(deltaTime);

	//衝突検出 → ダメージ解決(走査中はイベントを集めるだけで、走査後にまとめて適用する)
	collisionEvents_.clear();
	collisionAdapter_->Detect(collisionEvents_);
	const DamageResolver::Result damageResult = damageResolver_->Resolve(collisionEvents_);

	//撃破数だけ必殺技ゲージを加算する
	playerManager_->AddSpecialGauge(static_cast<int32_t>(damageResult.defeatedEnemies.size()));

	//死亡した敵・弾丸のColliderを解除してから削除
	enemyManager_->RemoveDead();
	bulletManager_->RemoveDead();

	if (playerManager_->IsDead()) {
		isGameOver_ = true;
	}
}

void ShootingGame::UpdateRenderers(float deltaTime) {
	// ゲームオーバー中は転がり等の見た目も止める
	const float renderDeltaTime = isGameOver_ ? 0.0f : deltaTime;

	playerRenderer_.Update(playerManager_->GetDrawState(), renderDeltaTime);

	enemyManager_->CollectDrawStates(enemyDrawStates_);
	enemyRenderer_.Update(enemyDrawStates_, renderDeltaTime);

	bulletManager_->CollectDrawStates(bulletDrawStates_);
	bulletRenderer_.Update(bulletDrawStates_);
}

void ShootingGame::Draw() {
	const Camera* camera = cameraController_.GetCamera();

	fieldRenderer_.Draw(directContext_, camera);
	enemyRenderer_.Draw(directContext_, camera);
	bulletRenderer_.Draw(directContext_, camera);
	playerRenderer_.Draw(directContext_, camera);
}

void ShootingGame::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("GameStatus");
	ImGui::Text("State : %s", isGameOver_ ? "GameOver" : "Playing");
	ImGui::Text("HP    : %d / %d", playerManager_->GetHp(), playerManager_->GetConfig().maxHp);
	ImGui::Text("Gauge : %d / %d", playerManager_->GetSpecialGauge(), playerManager_->GetConfig().specialGaugeMax);
	ImGui::Text("Enemy : %d", static_cast<int>(enemyManager_->GetEnemyCount()));
	ImGui::Text("Bullet: %d", static_cast<int>(bulletManager_->GetBulletCount()));
	ImGui::Text("Device: %s", keyManager_->GetActiveDevice() == InputDevice::Controller ? "Controller" : "Keyboard/Mouse");
	ImGui::End();
#endif
}

bool ShootingGame::UseSpecial() const {
	return playerManager_->IsSpecialActivated();
}

void ShootingGame::SaveConfigs() const {
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

PlayerInput ShootingGame::MakePlayerInput() const {
	auto key = keyManager_->GetKeyStates();

	PlayerInput input;
	input.fire = key[Key::Fire];
	input.special = key[Key::Special];

	// スティックはデッドゾーン適用後のアナログ量をそのまま使う(一律に長さ1へ正規化しない)
	const Vector2 stick = keyManager_->GetLeftStick();
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

AimInput ShootingGame::MakeAimInput() const {
	AimInput input;
	input.device = keyManager_->GetActiveDevice();
	input.cursorPosition = keyManager_->GetCursorPos();
	input.rightStick = keyManager_->GetRightStick();
	input.screenSize = screenSize_;
	return input;
}
