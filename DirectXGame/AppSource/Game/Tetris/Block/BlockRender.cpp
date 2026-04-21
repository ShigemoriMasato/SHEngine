#include "BlockRender.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <Game/Tetris/Mino/Tetrimino.h>

using namespace SHEngine;

BlockRender::~BlockRender() {
	Save();
}

void BlockRender::Initialize(uint32_t fieldWidth, uint32_t fieldHeight, Camera* camera, const DrawData& drawData) {
	logger_ = getLogger("Tetris");

	fieldWidth_ = fieldWidth;
	fieldHeight_ = fieldHeight;

	camera_ = camera;

	//Field
	uint32_t blockNum = fieldWidth_ * fieldHeight_;

	//Wall
	blockNum += (fieldHeight_ * 2) + (fieldWidth_ + 2);

	//Hold + Next(5個分)
	blockNum += 4 * 5;

	//GPU準備
	blockObject_ = std::make_unique<RenderObject>("Tetris Block");
	blockObject_->Initialize();

	blockObject_->psoConfig_.vs = "Game/Block.VS.hlsl";
	blockObject_->psoConfig_.ps = "Game/Block.PS.hlsl";

	blockObject_->SetDrawData(drawData);
	vertexDataIndex_ = blockObject_->CreateSRV(sizeof(VSData), blockNum, ShaderType::VERTEX_SHADER, "BlockRender::VSData");
	blockObject_->instanceNum_ = blockNum;

	//CPU準備
	vertexDatas_.resize(blockNum);

	//Binary
	binaryManager_ = std::make_unique<BinaryManager>();
	Load();

	//座標の初期化
	blockTransforms_.resize(blockNum);
	blockColors_.resize(blockNum);
	uint32_t index = 0;
	//Field
	for (int i = 0; i < int(fieldHeight_); ++i) {
		for (int j = 0; j < int((fieldWidth_)); ++j) {
			blockColors_[index] = colorMap_[0];
			blockTransforms_[index].position = Vector3(float(j - int(fieldWidth_) / 2), float(i - int(fieldHeight_) / 2), 0.0f);
			++index;
		}
	}

	//Wall
	uint32_t wallIndex = 8; //灰色
	for (int i = 0; i < int(fieldHeight_); ++i) {
		//左
		blockColors_[index] = colorMap_[wallIndex];
		blockTransforms_[index].position = Vector3(float(-int(fieldWidth_) / 2 - 1), float(i - int(fieldHeight_) / 2), 0.0f);
		++index;
		//右
		blockColors_[index] = colorMap_[wallIndex];
		blockTransforms_[index].position = Vector3(float(fieldWidth_ / 2), float(i - int(fieldHeight_) / 2), 0.0f);
		++index;
	}
	for (int i = 0; i < int(fieldWidth_) + 2; ++i) {
		//下
		blockColors_[index] = colorMap_[wallIndex];
		blockTransforms_[index].position = Vector3(float(i - int(fieldWidth_) / 2 - 1), float(-int(fieldHeight_) / 2 - 1), 0.0f);
		++index;
	}

	//DeleteEffect
	deleteEffect_ = std::make_unique<DeleteEffect>();
}

void BlockRender::Update(float deltaTime) {

	if (isDeleting_) {
		deleteEffect_->Update(deltaTime);

		Transform effectTransform = deleteEffect_->GetTransform();

		for (int line : deletingLines_) {
			for (uint32_t x = 0; x < fieldWidth_; ++x) {
				uint32_t index = static_cast<uint32_t>(line) * fieldWidth_ + x;
				blockTransforms_[index].scale = effectTransform.scale;
				blockTransforms_[index].rotate = effectTransform.rotate;
			}
		}

		if (deleteEffect_->ReqDelete()) {
			SetBlock(deletedField_, MovableMino{});
		}
		if (deleteEffect_->FinishEffect()) {
			isDeleting_ = false;
		}
	}

	for (size_t i = 0; i < blockTransforms_.size(); ++i) {
		VSData& data = vertexDatas_[i];
		data.worldMatrix = Matrix::MakeAffineMatrix(blockTransforms_[i].scale, blockTransforms_[i].rotate, blockTransforms_[i].position);
		data.vpMatrix = camera_->GetVPMatrix();
		data.color = blockColors_[i].first;
		data.outlineColor = blockColors_[i].second;
	}
	blockObject_->CopyBufferData(vertexDataIndex_, vertexDatas_.data(), sizeof(VSData) * vertexDatas_.size());
}

void BlockRender::SetStageData(std::vector<std::vector<int>> fieldData, const MovableMino& mino) {
	if(fieldData.empty()) {
		logger_->warn("BlockRender::SetStageData() fieldData is empty");
		return;
	}

	if (fieldData.size() < fieldHeight_ || fieldData[0].size() < fieldWidth_) {
		return;
	}

	for (int i = 0; i < int(fieldHeight_); ++i) {
		for (int j = 0; j < int(fieldWidth_); ++j) {
			blockColors_[i * fieldWidth_ + j] = colorMap_[fieldData[i][j]];
		}
	}

	for (size_t i = 0; i < mino.offset.size(); ++i) {
		int x = mino.offset[i].first + mino.position.first;
		int y = mino.offset[i].second + mino.position.second;
		if (x < 0 || x >= int(fieldWidth_) || y < 0 || y >= int(fieldHeight_)) {
			continue;
		}
		blockColors_[y * fieldWidth_ + x] = colorMap_[mino.minoType];
	}
}

void BlockRender::SetHoldMino(std::vector<std::pair<int, int>> blockPos, int colorID) {
	int i;
	uint32_t startIndex = fieldWidth_ * fieldHeight_ + (fieldHeight_ * 2) + (fieldWidth_ + 2);
	for (i = 0; i < int(blockPos.size()); ++i) {
		blockTransforms_[startIndex + i].position = holdBasePosition_ + Vector3(float(blockPos[i].first), float(blockPos[i].second), 0.0f);
		blockColors_[startIndex + i] = colorMap_[colorID];
	}

	for(i; i < 4; ++i) {
		blockColors_[startIndex + i] = colorMap_[0];
	}
}

void BlockRender::SetNextMino(std::vector<std::pair<int, int>> blockPos, int colorID) {
	if (blockPos.size() < 4) {
		logger_->warn("BlockRender::SetNextMino() blockPos size is too small");
	}
	if (blockPos.size() > 4 * 5) {
		logger_->warn("BlockRender::SetNextMino() blockPos size is too large");
	}

	uint32_t startIndex = fieldWidth_ * fieldHeight_ + (fieldHeight_ * 2) + (fieldWidth_ + 2) + 4;
	int minoCount = -1;

	for (size_t i = 0; i < blockPos.size(); ++i) {
		if(i % 4 == 0) {
			++minoCount;
		}

		blockTransforms_[startIndex + i].position = nextBasePosition_ + nextGap_ * float(minoCount) + Vector3(float(blockPos[i].first), float(blockPos[i].second), 0.0f);
		blockColors_[startIndex + i] = colorMap_[colorID];
	}
}

void BlockRender::SetBlock(int x, int y, int configIndex) {
	int index = y * fieldWidth_ + x;
	if (colorMap_.find(configIndex) == colorMap_.end()) {
		logger_->warn("BlockRender::SetBlock() configIndex {} not found in colorMap", configIndex);
	}

	blockColors_[index] = colorMap_[configIndex];
}

void BlockRender::SetBlock(std::vector<std::vector<int>> allConfigIndices, MovableMino movableMino) {
#ifdef SH_DEBUG
	logger_->debug("BlockRender::SetBlock() called");
	std::string debugStr = "Pre Stage Data:\n";
	for (const auto& row : allConfigIndices) {
		for (const auto& val : row) {
			debugStr += std::to_string(val) + " ";
		}
		debugStr += "\n";
	}
	logger_->debug(debugStr);
#endif

	if (allConfigIndices.size() < static_cast<size_t>(fieldHeight_) ||
		allConfigIndices[0].size() < static_cast<size_t>(fieldWidth_)) {
		return; // サイズが足りない場合は何もしない
	}

	for (const auto& [x, y] : movableMino.offset) {
		int posX = x + movableMino.position.first;
		int posY = y + movableMino.position.second;
		allConfigIndices[posY][posX] = movableMino.minoType;
	}

	for (uint32_t y = 0; y < fieldHeight_; ++y) {
		for (uint32_t x = 0; x < fieldWidth_; ++x) {
			int configIndex = allConfigIndices[y][x];
			SetBlock(x, y, configIndex);
		}
	}

#ifdef SH_DEBUG
	debugStr = "Now Stage Data:\n";
	for (const auto& row : allConfigIndices) {
		for (const auto& val : row) {
			debugStr += std::to_string(val) + " ";
		}
		debugStr += "\n";
	}
	logger_->debug(debugStr);
#endif
}

void BlockRender::BeginDeleteEffect(std::vector<int> fillLines, std::vector<std::vector<int>> deletedField) {
	deletedField_ = deletedField;
	deletingLines_ = fillLines;
	isDeleting_ = true;
	deleteEffect_->Initialize();
}

void BlockRender::Draw(CmdObj* cmdObj) {
	blockObject_->Draw(cmdObj);
}

void BlockRender::DrawImGui() {
#ifdef USE_IMGUI
	if (ImGui::Begin("BlockRender")) {
		ImGui::InputFloat3("HoldBasePosition", &holdBasePosition_.x);
		ImGui::InputFloat3("NextBasePosition", &nextBasePosition_.x);
		ImGui::InputFloat3("NextGap", &nextGap_.x);
		if (ImGui::Button("Save")) {
			Save();
		}
		if (ImGui::Button("Load")) {
			Load();
		}
	}
	ImGui::End();

	if (ImGui::Begin("ColorMap")) {
		//いじるMapを選択
		ImGui::Text("ID / %d", colorMapEditID_);
		ImGui::SameLine();
		if (ImGui::Button("-")) {
			colorMapEditID_ = std::max(0, colorMapEditID_ - 1);
		}
		ImGui::SameLine();
		if (ImGui::Button("+")) {
			colorMapEditID_ = std::min(static_cast<int>(colorMap_.size() - 1), colorMapEditID_ + 1);
		}

		//色編集
		Vector4 colorBuffer = ConvertColor(colorMap_[colorMapEditID_].first);
		ImGui::ColorEdit4("Color", &colorBuffer.x);
		colorMap_[colorMapEditID_].first = ConvertColor(colorBuffer);

		colorBuffer = ConvertColor(colorMap_[colorMapEditID_].second);
		ImGui::ColorEdit4("OutlineColor", &colorBuffer.x);
		colorMap_[colorMapEditID_].second = ConvertColor(colorBuffer);
	}
	ImGui::End();
#endif
}


// Binary関係 ==================================================

void BlockRender::Save() {
	binaryManager_->Register<Vector3>(&holdBasePosition_);
	binaryManager_->Register<Vector3>(&nextBasePosition_);
	binaryManager_->Register<Vector3>(&nextGap_);

	for (const auto& [id, colors] : colorMap_) {
		binaryManager_->Register<int>(&id);
		binaryManager_->Register<uint32_t>(&colors.first);
		binaryManager_->Register<uint32_t>(&colors.second);
	}

	binaryManager_->Write(fileName_);
}

void BlockRender::Load() {
	if (!binaryManager_->Boot(fileName_)) {
		//ファイルを開けなかった場合は初期値を入力
		colorMap_ = {
		{ int(Tetrimino::Type::None), {0x00000, 0x00000000} }, // 空白		Air
		{ int(Tetrimino::Type::S), {0x00ff00ff, 0xffffffff} }, // 赤			S
		{ int(Tetrimino::Type::Z), {0xff0000ff, 0xffffffff} }, // 緑			Z
		{ int(Tetrimino::Type::T), {0xff00ffff, 0xffffffff} }, // 紫			T
		{ int(Tetrimino::Type::O), {0xffff00ff, 0xffffffff} }, // 黄			O
		{ int(Tetrimino::Type::I), {0x00ffffff, 0xffffffff} }, // 水色		I
		{ int(Tetrimino::Type::L), {0x0000ffff, 0xffffffff} }, // 青			L
		{ int(Tetrimino::Type::J), {0xff8000ff, 0xffffffff} }, // オレンジ	J
		{ int(Tetrimino::Type::Wall), {0x808080ff, 0x000000ff} }, // 灰色	Wall
		{ int(Tetrimino::Type::Del), {0xffffffff, 0xffffffff} },// 白		DeleteEffect
		{ int(Tetrimino::Type::Hold), {0x000000ff, 0xffffffff} },
		{ int(Tetrimino::Type::Next), {0x000000ff, 0xffffffff} }
		};
		return;
	}

	holdBasePosition_ = binaryManager_->Reverse<Vector3>();
	nextBasePosition_ = binaryManager_->Reverse<Vector3>();
	nextGap_ = binaryManager_->Reverse<Vector3>();

	for (int i = 0; i < int(Tetrimino::Count); ++i) {
		int id = binaryManager_->Reverse<int>();
		uint32_t color = binaryManager_->Reverse<uint32_t>();
		uint32_t outlineColor = binaryManager_->Reverse<uint32_t>();

		//未設定な場合は上書きしない
		if(color == 0 && outlineColor == 0) {
			continue;
		}

		colorMap_[static_cast<int>(i)] = { color, outlineColor };
	}
}
