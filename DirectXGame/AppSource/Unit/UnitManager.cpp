#include"UnitManager.h"

void UnitManager::Initalize(MapChipField* mapChipField, DrawData playerDrawData, DrawData oreDrawData, KeyManager* keyManager) {
	// マップデータを取得
	mapChipField_ = mapChipField;

	// プレイヤーユニットを初期化
	playerUnit_ = std::make_unique<PlayerUnit>();
	playerUnit_->Initialize(mapChipField_, playerDrawData,{3.0f,0.0f,3.0f}, keyManager);

	// おれのモデルデータを取得
	oreDrawData_ = oreDrawData;

	// おれのメモリを確保
	oreUnits_.reserve(maxOreCount_);

	// 初期化
	AddOreUnit({ 1.0f,0.0f,1.0f });
}

void UnitManager::Update() {

	// プレイヤーの更新処理
	playerUnit_->Update();

	// おれユニットの更新処理
	//for (const auto& [id, unit] : oreUnits_) {
	//	unit->Update();
	//}
}

void UnitManager::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// プレイヤーを描画
	playerUnit_->Draw(window, vpMatrix);

	// おれを描画
	//for (const auto& [id, unit] : oreUnits_) {
	//	unit->Draw(window, vpMatrix);
	//}
}

void UnitManager::AddOreUnit(const Vector3& pos) {

	int32_t index;
	if (!freeIndices_.empty()) {
		index = freeIndices_.front();
		freeIndices_.pop_front();
	} else {
		if (currentId_ >= maxOreCount_) {
			return;
		}
		index = currentId_++;
	}

	// 登録
	std::unique_ptr<OreUnit> oreUnit = std::make_unique<OreUnit>();
	oreUnit->Initialize(mapChipField_, oreDrawData_, pos);

	oreUnits_[index] = std::move(oreUnit);
}