#include"PlayerUnit.h"
#include"FpsCount.h"

#ifdef USE_IMGUI
#include <imgui/imgui.h>
#endif

void PlayerUnit::Initialize(MapChipField* mapChipField, DrawData drawData, const Vector3& pos, KeyManager* keyManager) {
	// マップデータ
	mapChipField_ = mapChipField;

	// 入力を取得
	keyManager_ = keyManager;

	// オブジェクトを初期化
	object_ = std::make_unique<PlayerUnitObject>();
	object_->Initialize(drawData);

	// 初期位置を設定
	object_->transform_.position = pos;
}

void PlayerUnit::Update() {

	// 入力処理
	ProcessMoveInput();

	// 衝突判定情報を初期化
	MapChipField::CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;
	collisionMapInfo.pos = object_->transform_.position;
	collisionMapInfo.size = size_;

	// マップ衝突判定
	CheckMapCollision(collisionMapInfo);

	// 移動
	object_->transform_.position += collisionMapInfo.move;

	// 更新処理
	object_->Update();
}

void PlayerUnit::Draw(Window* window, const Matrix4x4& vpMatrix) {
	// 描画
	object_->Draw(window, vpMatrix);

	ImGui::Begin("Player");

	ImGui::DragFloat3("PlayerPos", &object_->transform_.position.x);

	ImGui::End();
}

void PlayerUnit::ProcessMoveInput() {

	// 速度
	velocity_ = { 0.0f,0.0f,0.0f };

	// 操作
	auto key = keyManager_->GetKeyStates();

	if (key[Key::Up]) {
		velocity_.z = speed_ * FpsCount::deltaTime;
	}

	if (key[Key::Down]) {
		velocity_.z = -speed_ * FpsCount::deltaTime;
	}

	if (key[Key::Left]) {
		velocity_.x = -speed_ * FpsCount::deltaTime;
	}

	if (key[Key::Right]) {
		velocity_.x = speed_ * FpsCount::deltaTime;
	}
}

void PlayerUnit::CheckMapCollision(MapChipField::CollisionMapInfo& info) {
	// 上方向の衝突判定
	CheckCollisionAbove(info);
	// 下方向の衝突判定
	CheckCollisionBelow(info);
	// 右方向の衝突判定
	CheckCollisionRight(info);
	// 左方向の衝突判定
	CheckCollisionLeft(info);
}

void PlayerUnit::CheckCollisionAbove(MapChipField::CollisionMapInfo& info) {
	// 上移動がなければ早期リターン
	if (info.move.z <= 0.0f) {
		return;
	}

	// 上の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Above, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, info.move.z + size_.z / 2.0f));
		// 現在の座用が壁の外かを判定
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, +size_.z / 2.0f));
		if (indexSetNow.zIndex != indexSet.zIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
			info.move.z = std::max(0.0f, (rect.bottom - object_->transform_.position.z) - (size_.z / 2.0f));
			// 壁に当たったことを記録する
			info.isWallHit = true;
		}
	}
}

void PlayerUnit::CheckCollisionBelow(MapChipField::CollisionMapInfo& info) {
	// 下移動がなければ早期リターン
	if (info.move.z >= 0.0f) {
		return;
	}

	// 下の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Below, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, info.move.z - size_.z / 2.0f));
		// 現在の座用が壁の外かを判定
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(0.0f, 0.0f, -size_.z / 2.0f));
		if (indexSetNow.zIndex != indexSet.zIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
			info.move.z = std::min(0.0f, (rect.top - object_->transform_.position.z) * -1.0f + (size_.z / 2.0f));
			// 壁に当たったことを記録する
			info.isWallHit = true;
		}
	}
}

void PlayerUnit::CheckCollisionRight(MapChipField::CollisionMapInfo& info) {
	// 右移動がなければ早期リターン
	if (info.move.x <= 0.0f) {
		return;
	}

	// 右の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Right, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(info.move.x + size_.x / 2.0f, 0.0f, 0.0f));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.move.x = std::min(0.0f, (rect.left - object_->transform_.position.x) - (size_.x / 2.0f));
		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}

void PlayerUnit::CheckCollisionLeft(MapChipField::CollisionMapInfo& info) {
	// 左移動がなければ早期リターン
	if (info.move.x >= 0.0f) {
		return;
	}

	// 左の判定を取る
	if (mapChipField_->IsBlockHit(MapChipField::MoveDir::Left, info)) {
		// めり込みを排除する方向に移動量を設定
		MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(object_->transform_.position + Vector3(info.move.x - size_.x / 2.0f, 0.0f, 0.0f));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.zIndex);
		info.move.x = std::max(0.0f, (rect.right - object_->transform_.position.x) + (size_.x / 2.0f));
		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}