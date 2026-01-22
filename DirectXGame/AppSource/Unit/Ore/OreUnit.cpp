#include"OreUnit.h"
#include"FpsCount.h"
#include <Common/DebugParam/GameParamEditor.h>
#include"Utility/Easing.h"

#include"Item/OreItemStorageNum.h"
#include"Item/Object/GoldOre.h"

OreUnit::OreUnit(MapChipField* mapChipField, DrawData drawData, Vector3* playerPos) {
	// マップデータ
	mapChipField_ = mapChipField;

	// プレイヤー座標を取得
	playerPos_ = playerPos;

	// オブジェクトを初期化
	object_ = std::make_unique<OreUnitObject>();
	object_->Initialize(drawData);

	// 更新状態を設定する
	statesTable_ = {
		[this]() { GoToUpdate(); },
		[this]() { MiningUpdate(); },
		[this]() {ToDeliverUpdate(); },
		[this]() { ReturnUpdate(); }
	};

	// 各状態のリセット処理を設定する
	resetStatesTable_ = {
		[this]() {},
		[this]() {},
		[this]() {
			// プレイヤーの位置までの経路を取得
			CalculatePath(*playerPos_);
		},
		[this]() {
			returnPhase_ = ReturnPhase::Rise;
			timer_ = 0.0f;
			toRotPos_ = homePos_;
		}
	};

	// 当たり判定の設定
	circleCollider_.center = Vector2(object_->transform_.position.x, object_->transform_.position.z);
	circleCollider_.radius = 1.0f;

	// 当たり判定の要素を設定
	CollConfig config;
	config.colliderInfo = &circleCollider_;
	config.isActive = true;
	config.ownTag = CollTag::Unit;
	config.targetTag = static_cast<uint32_t>(CollTag::Player) | static_cast<uint32_t>(CollTag::Stage);
	SetColliderConfig(config);

	// 当たり判定の初期化
	Initialize();

#ifdef USE_IMGUI
	// 値の登録
	RegisterDebugParam();
#endif
	// 値の適応
	ApplyDebugParam();
}

void OreUnit::Init(const Vector3& apearPos, const Vector3& targetPos) {

	// 初期位置を設定
	object_->transform_.position = apearPos;
	object_->transform_.scale = { 0.5f,0.5f,0.5f };
	// 拠点位置を設定
	homePos_ = apearPos;

	// フラグをリセット
	isActive_ = true;
	isDead_ = false;
	isRemoveOre_ = false;

	// hpを設定
	hp_ = maxHp_;

	// 時間をリセット
	lifeTimer_ = 0.0f;

	// 状態を設定
	state_ = State::GoTo;
	// 状態のリセット
	resetStatesTable_[static_cast<size_t>(state_)]();

	// 移動する経路を求める
	CalculatePath(targetPos);
}

void OreUnit::Update() {
#ifdef USE_IMGUI
	// 値の適応
	ApplyDebugParam();
#endif

	if (isDead_) { return; }

	isHit = false;

	// 切り替え処理
	if (stateRequest_) {
		// 状態を変更
		state_ = stateRequest_.value();
		// 状態によるリセットを呼び出す
		resetStatesTable_[static_cast<size_t>(state_)]();
		// リクエストをクリア
		stateRequest_ = std::nullopt;
	}

	// プレイヤーの状態による更新処理をおこなう
	statesTable_[static_cast<size_t>(state_)]();

	// 移動処理
	Move();

	// 更新処理
	object_->Update();

	// 当たり判定の位置を更新
	circleCollider_.center = Vector2(object_->transform_.position.x, object_->transform_.position.z);

	// 帰宅していれば体力処理を飛ばす
	if (state_ == State::Return) { return; }

	// 生存時間
	lifeTimer_ += FpsCount::deltaTime / damageTime_;

	if (lifeTimer_ >= 1.0f) {
		hp_ -= 1;
		lifeTimer_ = 0.0f;
	}

	// 体力が0の時、死亡する
	if (hp_ <= 0) {
		isDead_ = true;
	}
}

void OreUnit::Draw(Window* window, const Matrix4x4& vpMatrix) {
	if (isDead_) { return; }

	// 描画
	object_->Draw(window, vpMatrix);
}

void OreUnit::OnCollision(Collider* other) {

	if (isDead_) { return; }
	if (!isActive_) { return; }
	isHit = true;

	bool isStage = CollTag::Stage == other->GetOwnTag();
	bool isPlayer = CollTag::Player == other->GetOwnTag();

	// プレイヤーとの当たり判定
	if (isPlayer) {
		if (state_ != State::ToDeliver) { return; }
		// プレイヤーに触れれば帰宅する

		if (isActive_) {
			// 鉱石を収納する
			OreItemStorageNum::currentOreItemNum_ += 1;
			// 納品したら帰宅する
			stateRequest_ = State::Return;
		}
	}

	// 鉱石との当たり判定
	if (isStage) {

		if (state_ != State::GoTo && state_ != State::Mining) { return; }

		GoldOre* goldOre = dynamic_cast<GoldOre*>(other);
		if (goldOre) {

			if (state_ == State::GoTo) {
				// 指定した鉱石に近ければ採掘に移る
				if (path_.size() > 3) { return; }

				// 採掘状態に切り替える
				stateRequest_ = State::Mining;
				//goldOre->GetContactNum();
				timer_ = 0.0f;
			} else if (state_ == State::Mining) {

				if (timer_ >= 0.8f) {

					if (!isRemoveOre_) {
						isRemoveOre_ = true;
						goldOre->RemoveWorker();
						goldOre->RemoveHp();
					}
				}
			} 
		}
	}
}

void OreUnit::GoToUpdate() {

	// 目的地付けば次の状態に切り替える
	if (path_.empty()) {
		// 目的地に到着して鉱石が存在していなかった場合、帰宅する
		stateRequest_ = State::ToDeliver;
	}

	// 鉱石まで移動する
	Move();

	// 進行方向に回転
	Rotate();
}

void OreUnit::MiningUpdate() {

	timer_ += FpsCount::deltaTime / miningTime_;

	if (timer_ >= 1.0f) {
		timer_ = 0.0f;
		stateRequest_ = State::ToDeliver;
	}
}

void OreUnit::ToDeliverUpdate() {
	// 回収が終われば帰宅する
	if (path_.empty()) {
		stateRequest_ = State::Return;
	}

	// プレイヤーが現在いるマップチップのインデックスと座標を取得
	MapChipField::IndexSet pIdx = mapChipField_->GetMapChipIndexSetByPosition(*playerPos_);
	Vector3 playerGridPos = mapChipField_->GetMapChipPositionByIndex(pIdx.xIndex, pIdx.zIndex);

	// 「現在の目的地（パスの終点）」と「プレイヤーのいるマスの中心」が違うかチェック
	bool needRecalc = false;
	if (!path_.empty()) {
		Vector3 currentDest = path_.back(); // 現在のパスの最終地点

		// 距離の二乗で比較
		float dx = currentDest.x - playerGridPos.x;
		float dz = currentDest.z - playerGridPos.z;
		if ((dx * dx + dz * dz) > 0.1f) {
			// 目的地が変わっているので再計算フラグを立てる
			needRecalc = true;
		}
	}

	// 再計算が必要なら実行
	if (needRecalc) {
		CalculatePath(*playerPos_);
	}

	// 移動する
	Move();

	// 進行方向に回転
	Rotate();
}

void OreUnit::ReturnUpdate() {

	switch (returnPhase_)
	{
	case ReturnPhase::Rise:

		timer_ += FpsCount::deltaTime / riseTime_;

		// 移動
		object_->transform_.position.y = lerp(0.0f, risePosY_, timer_, EaseType::EaseInOutQuad);

		// 回転移動
		Rotate();

		// 終了
		if (timer_ >= 1.0f) {
			object_->transform_.position.y = risePosY_;
			timer_ = 0.0f;

			// 拠点までのベクトルを求める
			Vector3 toHome = homePos_ - object_->transform_.position;
			toHome.y = 0.0f;
			toHome.Normalize();

			// 移動する位置を設定
			startMovePos = object_->transform_.position;
			endMovePos = Vector3(homePos_.x, risePosY_, homePos_.z);

			returnPhase_ = ReturnPhase::Move;
		}
		break;

	case ReturnPhase::Move:

		timer_ += FpsCount::deltaTime / moveTime_;

		// 移動
		object_->transform_.position = lerp(startMovePos, endMovePos, timer_, EaseType::EaseInOutQuad);

		// 終了
		if (timer_ >= 1.0f) {
			object_->transform_.position = endMovePos;
			timer_ = 0.0f;

			returnPhase_ = ReturnPhase::Fall;
		}
		break;

	case ReturnPhase::Fall:

		timer_ += FpsCount::deltaTime / FallTime_;

		// 移動
		object_->transform_.position.y = lerp(risePosY_, 0.0f, timer_, EaseType::EaseInOutQuad);

		// 終了
		if (timer_ >= 1.0f) {
			timer_ = 0.0f;
			isActive_ = false;
		}
		break;
	}
}

void OreUnit::CalculatePath(const Vector3& goal) {
	// メンバ変数のパスをクリア
	path_.clear();

	if (mapChipField_) {
		// マップクラスに経路計算を依頼
		// startPos は現在の自分の位置
		std::vector<Vector3> calculatedPath = mapChipField_->CalculatePath(object_->transform_.position, goal);
		path_ = calculatedPath;
	}

	// もし経路が見つからなかった場合
	if (path_.empty()) {
		// とりあえずゴール地点だけ入れておく
		path_.push_back(goal);
	}
}

void OreUnit::Move() {

	// パスに何も存在しなければ早期リターン
	if (path_.empty()) { return; }

	// 次に進むべき座標を取得
	Vector3 nextTarget = path_.front();
	Vector3 currentPos = object_->transform_.position;

	// 次の地点へのベクトルを計算
	Vector3 toTarget = nextTarget - currentPos;
	toTarget.y = 0.0f;

	// XZ平面での距離を計算
	float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z);

	// 移動処理
	if (distance < 0.1f) {
		// 十分近づいたら座標を確定させて、リストから削除（次の地点へ）
		object_->transform_.position = nextTarget;
		path_.erase(path_.begin());
	} else {
		// 目的地に向かって移動
		// 正規化（長さを1にする）して速度を掛ける
		Vector3 velocity = (toTarget / distance) * moveSpeed_;
		object_->transform_.position += velocity * FpsCount::deltaTime;

		// 進行方向を向かせる（Y軸回転）
		//if (distance > 0.001f) {
		//	object_->transform_.rotate.y = std::atan2(velocity.x, velocity.z);
		//}
	}
}

void OreUnit::Rotate() {
	// 回転処理
	if (!path_.empty()) {
		toRotPos_ = path_.front();
	}
	Vector3 nextTarget = toRotPos_;
	Vector3 toTarget = nextTarget - object_->transform_.position;
	toTarget.y = 0.0f;
	toTarget.Normalize();

	Vector3 targetRot = { 0, 0, 0 };
	// Y軸回転を取得
	targetRot.y = atan2f(toTarget.x, toTarget.z);
	Vector3 currentRot = object_->transform_.rotate;

	// 最短距離の角度を取得
	float diffY = GetShortAngleY(targetRot.y - currentRot.y);

	// 回転
	currentRot.y += diffY * rotateSpeed_ * FpsCount::deltaTime;
	object_->transform_.rotate = currentRot;
}

void OreUnit::RegisterDebugParam() {
	// 登録
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MoveSpeed", moveSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RotateSpeed", rotateSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxHp", maxHp_);	
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RiseHeight", risePosY_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RiseTime", riseTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MoveTime", moveTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "FallTime", FallTime_);
}

void OreUnit::ApplyDebugParam() {
	// 適応
	moveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MoveSpeed");
	rotateSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RotateSpeed");
	maxHp_ = GameParamEditor::GetInstance()->GetValue<int32_t>(kGroupName_, "MaxHp");
	risePosY_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RiseHeight");
	riseTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RiseTime");
	moveTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MoveTime");
	FallTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "FallTime");
}