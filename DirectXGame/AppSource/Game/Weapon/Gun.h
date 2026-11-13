#pragma once
#include "FireContext.h"

/// @brief 銃の操作インターフェース。プレイヤーは銃の具体型を知らず、このインターフェースだけを使う。
/// 銃は生成後の弾丸を保持・追跡せず、弾丸の更新・削除にも関与しない。
class Gun {
public:

	virtual ~Gun() = default;

	/// @brief クールダウン等の時間経過だけを更新する(秒)
	virtual void Update(float deltaTime) = 0;

	/// @brief 射撃要求。射撃入力が有効な間、毎フレーム呼ばれる。発射可能なら弾丸生成を依頼する
	virtual void Trigger(const FireContext& context) = 0;
};
