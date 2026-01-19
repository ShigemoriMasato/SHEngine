#include"MapChipRender.h"
#include"Utility/MatrixFactory.h"

void MapChipRender::Initialize(DrawData wallDrawData, std::vector<std::vector<TileType>> map) {

	// psoを設定
	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->Initialize();
	renderObject_->psoConfig_.inputLayoutID = InputLayoutID::Default;
	renderObject_->psoConfig_.vs = "Instancing3d.VS.hlsl";
	renderObject_->psoConfig_.ps = "Instancing3d.PS.hlsl";

	// 描画モデルを設定
	renderObject_->SetDrawData(wallDrawData);

	uint32_t maxNum = static_cast<uint32_t>(map.size() * map[0].size());

	// worldTransformを登録
	vsDataIndex_ = renderObject_->CreateSRV(sizeof(ParticleForGPU), maxNum, ShaderType::VERTEX_SHADER, "TestScene::SkinningMatrices");

	TransformData transformData;
	transformDatas_.reserve(maxNum);
	for (uint32_t i = 0; i < maxNum; ++i) {
		transformData.transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{} };
		transformData.worldMatrix = Matrix::MakeAffineMatrix(transformData.transform.scale, transformData.transform.rotate, transformData.transform.position);
		transformData.color = { 1.0f,1.0f,1.0f,1.0f };
		transformData.textureHandle = 0;
		transformDatas_.push_back(transformData);
	}

	// マップの位置を設定
	uint32_t n = 0;
	for (uint32_t z = 0; z < map.size(); ++z) {
		for (uint32_t x = 0; x < map[0].size(); ++x) {

			if (map[z][x] == TileType::Wall) {
				// 壁
				uint32_t i = n++;
				transformDatas_[i].transform.position = { 1.0f * x,0.0f,1.0f * z };
				transformDatas_[i].worldMatrix = Matrix::MakeAffineMatrix(transformDatas_[i].transform.scale, transformDatas_[i].transform.rotate, transformDatas_[i].transform.position);
				transformDatas_[i].color = { 1.0f,0.0f,1.0f,1.0f };
			} else if (map[z][x] == TileType::Road) {
				// 道
				uint32_t i = n++;
				transformDatas_[i].transform.position = { 1.0f * x,-1.0f,1.0f * z };
				transformDatas_[i].worldMatrix = Matrix::MakeAffineMatrix(transformDatas_[i].transform.scale, transformDatas_[i].transform.rotate, transformDatas_[i].transform.position);
				transformDatas_[i].color = { 1.0f,0.0f,0.0f,1.0f };
			} else if (map[z][x] == TileType::Home) {
				// 家
				uint32_t i = n++;
				transformDatas_[i].transform.position = { 1.0f * x,-1.0f,1.0f * z };
				transformDatas_[i].worldMatrix = Matrix::MakeAffineMatrix(transformDatas_[i].transform.scale, transformDatas_[i].transform.rotate, transformDatas_[i].transform.position);
				transformDatas_[i].color = { 1.0f,1.0f,0.0f,1.0f };
			}
		}
	}
	// 描画する数を設定
	currentNum_ = n;

	// 単位行列を書き込んでおく
	instancingData_.resize(transformDatas_.size());
	for (uint32_t index = 0; index < transformDatas_.size(); ++index) {
		instancingData_[index].WVP = Matrix4x4::Identity();
		instancingData_[index].World = Matrix4x4::Identity();
		instancingData_[index].color = { 1.0f,1.0f,1.0f,1.0f };
		instancingData_[index].textureHandle = 0;
	}

	// 値を設定
	renderObject_->CopyBufferData(vsDataIndex_, instancingData_.data(), sizeof(ParticleForGPU) * instancingData_.size());
}

void MapChipRender::Draw(Window* window, const Matrix4x4& vpMatrix) {

	// 描画個数
	renderObject_->instanceNum_ = currentNum_;

	// カメラの位置を適応
	for (uint32_t i = 0; i < renderObject_->instanceNum_; ++i) {
		instancingData_[i].WVP = transformDatas_[i].worldMatrix * vpMatrix;
		instancingData_[i].World = transformDatas_[i].worldMatrix;
		instancingData_[i].color = transformDatas_[i].color;
		instancingData_[i].textureHandle = transformDatas_[i].textureHandle;
	}

	// 値を設定
	renderObject_->CopyBufferData(vsDataIndex_, instancingData_.data(), sizeof(ParticleForGPU) * instancingData_.size());

	// 描画
	renderObject_->Draw(window);
}