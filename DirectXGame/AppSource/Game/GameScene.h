#pragma once
#include <Scene/IScene.h>
#include <Particle/ParticleSystem.h>
#include <Game/Field/BattleField.h>
#include <Game/Boss/Beam/ExplosionBeam.h>
#include <Render/PostEffect/PostEffectContext.h>
#include <Render/PostEffect/BloomEffect.h>

#include <Game/Aim/AimController.h>
#include <Game/Bullet/BulletManager.h>
#include <Game/Camera/GameCameraController.h>
#include <Game/Collision/CollisionAdapter.h>
#include <Game/Collision/DamageResolver.h>
#include <Game/Common/FieldConfig.h>
#include <Game/Enemy/EnemyManager.h>
#include <Game/Player/PlayerManager.h>
#include <Game/Render/BulletRenderer.h>
#include <Game/Render/EnemyRenderer.h>
#include <Game/Render/FieldRenderer.h>
#include <Game/Render/PlayerRenderer.h>
#include <Game/Weapon/GunManager.h>

/// @brief ゲーム全体の組み立て役。各Managerの生成・依存注入・更新順・描画呼び出し・終了判定をまとめる。
///
/// 1フレームの更新順:
///  (1) KeyManager入力更新と操作デバイス判定
///  (2) Playerの入力・移動更新
///  (3) Cameraの追従更新
///  (4) AimControllerが更新後のCamera行列から照準を作成
///  (5) EnemyManagerが敵を更新・生成
///  (6) GunManagerがクールダウンを更新
///  (7) Key::FireならPlayerがGun::Triggerを呼ぶ
///  (8) BulletManagerが弾丸を更新(同フレームに生成した弾丸は次フレームから更新)
///  (9) 衝突検出とダメージ解決
///  (10) 死亡物と関連Colliderを削除
///  (11) HPからゲーム状態を遷移(ゲームオーバー後は(2)～(10)を止める)
class GameScene : public IScene {
public:

	~GameScene() override;

	void Initialize() override;
	std::unique_ptr<IScene> Update() override;
	void Draw() override;

private:

	enum class GameState {
		Playing,
		GameOver,
	};

	void InitializeGame();
	void UpdateGame(float deltaTime);
	void UpdateRenderers(float deltaTime);
	void DrawGame();
	void DrawGameImGui();
	void SaveGameConfigs() const;

	/// @brief KeyManagerの結果をゲーム用の入力へ変換する
	PlayerInput MakePlayerInput() const;
	AimInput MakeAimInput() const;

	// 1フレームのdeltaTimeの上限(秒)。ヒッチ時の大移動を防ぐ
	static constexpr float kMaxDeltaTime = 1.0f / 20.0f;

	// ゲームカメラ(ParticleSystemからも参照されるため、ParticleSystemより先に宣言する)
	GameCameraController cameraController_;

	ImGuiWindowManager imguiManager_;

	// 1,048,576 slots for BattleField plus 4,194,304 for ExplosionBeam.
	ParticleSystem particleSystem_ = { 5242880, true };

	std::unique_ptr<BattleField> battleField_ = nullptr;
	std::unique_ptr<ExplosionBeam> explosionBeam_ = nullptr;

	ExplosionBeamConfig beamConfig_;

	PostEffectContext postEffectContext_;
	BloomEffect bloomEffect_;
	Bloom bloom_;

	// ================- ゲーム -================
	// 宣言順 = 生成順。メンバは宣言の逆順に破棄されるため、参照される側を先に宣言する。
	//   FieldConfig > CollisionAdapter > BulletManager > GunManager > PlayerManager / EnemyManager > DamageResolver
	// (PlayerはGunを、GunはBulletManagerを、各ManagerはCollisionAdapterとFieldConfigを非所有で参照する)

	FieldConfig fieldConfig_{};
	AimController aimController_;

	std::unique_ptr<CollisionAdapter> collisionAdapter_ = nullptr;
	std::unique_ptr<BulletManager> bulletManager_ = nullptr;
	std::unique_ptr<GunManager> gunManager_ = nullptr;
	std::unique_ptr<PlayerManager> playerManager_ = nullptr;
	std::unique_ptr<EnemyManager> enemyManager_ = nullptr;
	std::unique_ptr<DamageResolver> damageResolver_ = nullptr;

	// 描画(値型の描画スナップショットだけを受け取る)
	PlayerRenderer playerRenderer_;
	EnemyRenderer enemyRenderer_;
	BulletRenderer bulletRenderer_;
	FieldRenderer fieldRenderer_;

	// フレーム内の作業用バッファ
	std::vector<CollisionEvent> collisionEvents_;
	std::vector<EnemyDrawState> enemyDrawStates_;
	std::vector<BulletDrawState> bulletDrawStates_;

	GameState gameState_ = GameState::Playing;
};
