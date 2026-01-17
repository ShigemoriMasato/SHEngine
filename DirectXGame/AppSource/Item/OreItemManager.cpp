#include"OreItemManager.h"

// 各鉱石
#include"Object/GoldOre.h"

void OreItemManager::Initialize(const DrawData& goldOreDrawData) {

	// 金鉱石の描画データを取得
	goldOreDrawData_ = goldOreDrawData;

	// メモリを確保
	oreItems_.reserve(10);


}

void OreItemManager::Update() {

	// 鉱石の更新処理
	for (auto& ore : oreItems_) {
		ore->Update();
	}
}

void OreItemManager::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// 鉱石を描画
	for (auto& ore : oreItems_) {
		ore->Draw(window, vpMatrix);
	}
}

void OreItemManager::AddOreItem(OreType type, const Vector3& pos) {

	switch (type)
	{
	case OreType::Gold:
		// 金鉱石を追加
		std::unique_ptr<GoldOre> ore = std::make_unique<GoldOre>();
		ore->Initialize(goldOreDrawData_, pos);

		oreItems_.push_back(std::move(ore));
		break;
	}
}