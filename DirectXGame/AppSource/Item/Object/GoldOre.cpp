#include"GoldOre.h"

void GoldOre::Initialize(DrawData drawData, const Vector3& pos) {
	
	// オブジェクトを初期化
	object_ = std::make_unique<OreObject>();
	object_->Initialize(drawData);

	// 初期位置を設定
	object_->transform_.position = pos;
}

void GoldOre::Update() {



	// 更新処理
	object_->Update();
}

void GoldOre::Draw(Window* window, const Matrix4x4& vpMatrix) {
	// 描画
	object_->Draw(window, vpMatrix);
}