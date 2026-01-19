#pragma once
#include <Render/RenderObject.h>
#include"Render/DrawDataManager.h"
#include"Utility/DataStructures.h"

class PlayerUnitObject {
public:

	struct TransformationMatrix {
		Matrix4x4 WVP = Matrix4x4::Identity();
		Matrix4x4 World = Matrix4x4::Identity();
		Matrix4x4 worldInverseTranspose = Matrix4x4::Identity();
	};

	struct Material {
		Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	};

	// 初期化
	void Initialize(DrawData drawData);

	// 更新処理
	void Update();

	// 描画
	void Draw(Window* window, const Matrix4x4& vpMatrix);

public:

	// srt
	Transform transform_;

private:

	//　オブジェクトデータ
	std::unique_ptr<RenderObject> renderObject_;

	// worldTransform関係
	TransformationMatrix vsData_{};
	int vsDataIndex_ = -1;
	Matrix4x4 worldMatrix_;

	// マテリアル関係
	Material material_{};
	int psDataIndex_ = -1;
};