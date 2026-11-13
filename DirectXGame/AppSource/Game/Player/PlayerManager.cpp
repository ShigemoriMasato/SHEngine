#include "PlayerManager.h"
#include <Game/Collision/CollisionAdapter.h>
#include <Game/Common/ConfigFile.h>
#include <Game/Common/FieldConfig.h>
#include <Game/Common/GameMath.h>

PlayerManager::PlayerManager(CollisionAdapter* collision, const FieldConfig* field)
	: collision_(collision), field_(field) {
	player_ = std::make_unique<Player>(field_);
}

PlayerManager::~PlayerManager() {
	collision_->Unregister(BodyKind::Player, kPlayerBodyID);
}

void PlayerManager::Initialize(Gun* gun) {
	ConfigFile::Load(PlayerConfig::kFileName, config_);

	player_->Initialize(config_, field_->center);
	player_->SetGun(gun);

	collision_->Register(BodyKind::Player, kPlayerBodyID, player_->GetRadius());
	collision_->UpdateBody(BodyKind::Player, kPlayerBodyID, player_->GetPreviousPosition(), player_->GetPosition(), player_->GetRadius());
}

void PlayerManager::Update(float deltaTime, const PlayerInput& input) {
	player_->Update(deltaTime, input);
	collision_->UpdateBody(BodyKind::Player, kPlayerBodyID, player_->GetPreviousPosition(), player_->GetPosition(), player_->GetRadius());
}

void PlayerManager::Shot(const PlayerInput& input, const Vector2& aimDirection) {
	player_->Shot(input, aimDirection);
}

bool PlayerManager::RequestDamage(int32_t amount) {
	return player_->RequestDamage(amount);
}

bool PlayerManager::ApplyPendingDamage() {
	return player_->ApplyPendingDamage();
}

Vector3 PlayerManager::GetWorldPosition() const {
	return GameMath::ToWorld(player_->GetPosition(), field_->floorHeight + player_->GetRadius());
}

PlayerDrawState PlayerManager::GetDrawState() const {
	return player_->GetDrawState(field_->floorHeight);
}

void PlayerManager::SetConfig(const PlayerConfig& config) {
	config_ = config;
	config_.Validate();
	player_->SetConfig(config_);
}

void PlayerManager::SaveConfig() const {
	ConfigFile::Save(PlayerConfig::kFileName, config_);
}
