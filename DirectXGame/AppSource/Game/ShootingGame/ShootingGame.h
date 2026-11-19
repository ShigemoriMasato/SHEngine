#pragma once
#include <SHEngine.h>
#include <Utility/Vector.h>
#include <memory>
#include <vector>

#include <Game/ShootingGame/Aim/AimController.h>
#include <Game/ShootingGame/Bullet/BulletManager.h>
#include <Game/ShootingGame/Camera/GameCameraController.h>
#include <Game/ShootingGame/Collision/CollisionAdapter.h>
#include <Game/ShootingGame/Collision/DamageResolver.h>
#include <Game/ShootingGame/Common/FieldConfig.h>
#include <Game/ShootingGame/Enemy/EnemyManager.h>
#include <Game/ShootingGame/Field/BattleField.h>
#include <Game/ShootingGame/Player/PlayerManager.h>
#include <Game/ShootingGame/Render/BulletRenderer.h>
#include <Game/ShootingGame/Render/EnemyRenderer.h>
#include <Game/ShootingGame/Render/FieldRenderer.h>
#include <Game/ShootingGame/Render/PlayerRenderer.h>
#include <Game/ShootingGame/Weapon/GunManager.h>

class KeyManager;
class ParticleSystem;
class ImGuiWindowManager;

/// @brief シューティングゲーム本体。Player・Enemy・Weapon・Bullet・Fieldと、それに付随する
/// カメラ・照準・当たり判定・描画の生成・依存注入・更新順・描画呼び出し・終了判定をまとめる。
/// KeyManager・ParticleSystem・PostEffect・ImGuiWindowManagerなどゲームに直接関係しないものは所有せず、外部から受け取る。
class ShootingGame {
public:

	/// @param keyManager 入力取得用(非所有, 更新は呼び出し側)
	/// @param particleSystem BattleFieldのEmitter生成先(非所有, Initialize済みであること)
	/// @param imguiManager BattleFieldの調整ウィンドウ登録先(非所有)
	ShootingGame(SHEngine::Engine* engine, const KeyManager* keyManager, ParticleSystem* particleSystem, ImGuiWindowManager* imguiManager);
	~ShootingGame();

	/// @param screenSize 描画先の解像度
	void Initialize(const Vector2& screenSize);

	/// @brief ゲーム進行とフィールドの更新
	void Update(float deltaTime);

	/// @brief フィールドのテクスチャ描画・適用(ParticleSystem::Updateの後に呼ぶ)
	void ApplyField();

	/// @brief 描画スナップショットをRendererへ渡す(ゲーム状態はRendererから変更しない)
	void UpdateRenderers(float deltaTime);

	void Draw();
	void DrawImGui();

	/// @brief ゲームカメラ(ParticleSystemの描画にも使う)
	const Camera* GetCamera() const { return cameraController_.GetCamera(); }

	bool UseSpecial() const;


private:

	void UpdateGame(float deltaTime);
	void SaveConfigs() const;

	/// @brief KeyManagerの結果をゲーム用の入力へ変換する
	PlayerInput MakePlayerInput() const;
	AimInput MakeAimInput() const;

	// ================- 外部(非所有) -================
	SHEngine::Engine* engine_ = nullptr;
	SHEngine::DirectCommandContext* directContext_ = nullptr;
	SHEngine::ComputeCommandContext* computeContext_ = nullptr;
	const KeyManager* keyManager_ = nullptr;
	ParticleSystem* particleSystem_ = nullptr;
	ImGuiWindowManager* imguiManager_ = nullptr;

	Vector2 screenSize_ = { 1280.0f, 720.0f };

	// ================- ゲーム -================
	// 宣言順 = 生成順。メンバは宣言の逆順に破棄されるため、参照される側を先に宣言する。
	//   FieldConfig > CollisionAdapter > BulletManager > GunManager > PlayerManager / EnemyManager > DamageResolver
	// (PlayerはGunを、GunはBulletManagerを、各ManagerはCollisionAdapterとFieldConfigを非所有で参照する)

	GameCameraController cameraController_;

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

	bool isGameOver_ = false;
};
