#include"OreUnit.h"
#include"FpsCount.h"

void OreUnit::Initialize(MapChipField* mapChipField, DrawData drawData, const Vector3& apearPos, const Vector3& targetPos) {
	// マップデータ
	mapChipField_ = mapChipField;

	// オブジェクトを初期化
	object_ = std::make_unique<OreUnitObject>();
	object_->Initialize(drawData);

	// 初期位置を設定
	object_->transform_.position = apearPos;

    // 移動する経路を求める
    CalculatePath(targetPos);
}

void OreUnit::Update() {

    // 移動処理
    Move();

	// 更新処理
	object_->Update();
}

void OreUnit::Draw(Window* window, const Matrix4x4& vpMatrix) {
	// 描画
	object_->Draw(window, vpMatrix);
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
	// 1. 経路がない（目的地に到着した）場合の処理
	if (path_.empty()) {
		if (state_ == State::GoTo) {
			// 目的地に着いたので、家に帰るモードに切り替え
			state_ = State::Return;
			//CalculatePath(homePos_);
		} else if (state_ == State::Return) {
			// 家に着いたので死亡（削除）
			//state_ = State::Finished;
			isDead_ = true;
		}
		return;
	}

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
		Vector3 velocity = (toTarget / distance) * speed_;
		object_->transform_.position += velocity * FpsCount::deltaTime;

		// 進行方向を向かせる（Y軸回転）
		if (distance > 0.001f) {
			object_->transform_.rotate.y = std::atan2(velocity.x, velocity.z);
		}
	}
}