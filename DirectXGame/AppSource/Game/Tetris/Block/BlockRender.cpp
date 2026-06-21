#include "BlockRender.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <Game/Tetris/Mino/Tetrimino.h>
#include <Utility/Color.h>

using namespace SHEngine;

BlockRender::~BlockRender() {
	Save();
}

void BlockRender::Initialize(uint32_t fieldWidth, uint32_t fieldHeight, Camera* camera, const DrawData& drawData, SHEngine::TextureData* ddsTexture) {
	logger_ = GetLogger("Tetris");

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
	blockRenderer_ = std::make_unique<SHEngine::Renderer>(drawData);
	container_ = std::make_unique<SHEngine::BufferContainer>();

	bool reflect = true;

	blockRenderer_->SetVS("Game/ReflectBlock.VS.hlsl");
	blockRenderer_->SetPS("Game/ReflectBlock.PS.hlsl");

	vsBuffer_ = container_->Create(BufferType::SRV, sizeof(VSData), blockNum);
	//数適当
	colorMapBuffer_ = container_->Create(BufferType::SRV, sizeof(ColorMap), 32);
	psBuffer_ = container_->Create(BufferType::CBV, sizeof(PSData));
	auto ddsBuffer = container_->Create(ddsTexture);

	blockRenderer_->SetGPUBuffer(vsBuffer_, ShaderType::VERTEX_SHADER, BufferType::SRV);
	blockRenderer_->SetGPUBuffer(psBuffer_, ShaderType::PIXEL_SHADER, BufferType::CBV);
	blockRenderer_->SetGPUBuffer(colorMapBuffer_, ShaderType::PIXEL_SHADER, BufferType::SRV);
	blockRenderer_->SetGPUBuffer(ddsBuffer, ShaderType::PIXEL_SHADER, BufferType::DDSTexture);

	blockRenderer_->instanceNum_ = blockNum;

	//CPU準備
	vsData_.resize(blockNum);

	//Binary
	binaryManager_ = std::make_unique<BinaryManager>();
	Load();

	//座標の初期化
	blockTransforms_.resize(blockNum);
	uint32_t index = 0;
	//Field
	for (int i = 0; i < int(fieldHeight_); ++i) {
		for (int j = 0; j < int((fieldWidth_)); ++j) {
			vsData_[index].colorID = 0;
			blockTransforms_[index].position = Vector3(float(j - int(fieldWidth_) / 2), float(i - int(fieldHeight_) / 2), 0.0f);
			++index;
		}
	}

	//Wall
	constexpr uint32_t wallIndex = 8; //灰色
	for (int i = 0; i < int(fieldHeight_); ++i) {
		//左
		vsData_[index].colorID = wallIndex;
		blockTransforms_[index].position = Vector3(float(-int(fieldWidth_) / 2 - 1), float(i - int(fieldHeight_) / 2), 0.0f);
		++index;
		//右
		vsData_[index].colorID = wallIndex;
		blockTransforms_[index].position = Vector3(float(fieldWidth_ / 2), float(i - int(fieldHeight_) / 2), 0.0f);
		++index;
	}
	for (int i = 0; i < int(fieldWidth_) + 2; ++i) {
		//下
		vsData_[index].colorID = wallIndex;
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
		VSData& data = vsData_[i];
		data.world = Matrix::MakeAffineMatrix(blockTransforms_[i].scale, blockTransforms_[i].rotate, blockTransforms_[i].position);
		data.wvp = data.world * camera_->GetVPMatrix();
	}
	vsBuffer_->CopyBuffer(vsData_.data(), sizeof(VSData) * vsData_.size());

	colorMapBuffer_->CopyBuffer(colorMap_.data(), sizeof(ColorMap) * colorMap_.size());

	psData_.cameraPos = camera_->GetPosition();
	psBuffer_->CopyBuffer(&psData_, sizeof(psData_));
}

void BlockRender::SetStageData(std::vector<std::vector<int>> fieldData, const MovableMino& mino) {
	if (fieldData.empty()) {
		logger_->warn("BlockRender::SetStageData() fieldData is empty");
		return;
	}

	if (fieldData.size() < fieldHeight_ || fieldData[0].size() < fieldWidth_) {
		return;
	}

	for (int i = 0; i < int(fieldHeight_); ++i) {
		for (int j = 0; j < int(fieldWidth_); ++j) {
			vsData_[i * fieldWidth_ + j].colorID = fieldData[i][j];
		}
	}

	for (size_t i = 0; i < mino.offset.size(); ++i) {
		int x = mino.offset[i].first + mino.position.first;
		int y = mino.offset[i].second + mino.position.second;
		if (x < 0 || x >= int(fieldWidth_) || y < 0 || y >= int(fieldHeight_)) {
			continue;
		}
		vsData_[y * fieldWidth_ + x].colorID = mino.minoType;
	}
}

void BlockRender::SetHoldMino(std::vector<std::pair<int, int>> blockPos, int colorID) {
	int i;
	uint32_t startIndex = fieldWidth_ * fieldHeight_ + (fieldHeight_ * 2) + (fieldWidth_ + 2);
	for (i = 0; i < int(blockPos.size()); ++i) {
		blockTransforms_[startIndex + i].position = holdBasePosition_ + Vector3(float(blockPos[i].first), float(blockPos[i].second), 0.0f);
		vsData_[startIndex + i].colorID = colorID;
	}

	for (i; i < 4; ++i) {
		vsData_[startIndex + i].colorID = 0;
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
		if (i % 4 == 0) {
			++minoCount;
		}

		blockTransforms_[startIndex + i].position = nextBasePosition_ + nextGap_ * float(minoCount) + Vector3(float(blockPos[i].first), float(blockPos[i].second), 0.0f);
		vsData_[startIndex + i].colorID = colorID;
	}
}

void BlockRender::SetBlock(int x, int y, int configIndex) {
	int index = y * fieldWidth_ + x;
	assert(colorMap_.size() > configIndex);
	configIndex = std::min(int(colorMap_.size() - 1), configIndex);

	vsData_[index].colorID = configIndex;
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

void BlockRender::Draw(DCC* cmdObj) {
	blockRenderer_->Draw(cmdObj);
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
		ImGui::ColorEdit4("Color", &colorMap_[colorMapEditID_].color.x);

		ImGui::ColorEdit4("OutlineColor", &colorMap_[colorMapEditID_].outlineColor.x);

		ImGui::DragFloat("Reflect", &psData_.strength, 0.01f, 0.0f, 1.0f);
	}
	ImGui::End();
#endif
}


// Binary関係 ==================================================

void BlockRender::Save() {
	binaryManager_->Register<Vector3>(&holdBasePosition_);
	binaryManager_->Register<Vector3>(&nextBasePosition_);
	binaryManager_->Register<Vector3>(&nextGap_);

	for (int i = 0; i < colorMap_.size(); ++i) {
		binaryManager_->Register<int>(&i);
		binaryManager_->Register<Vector4>(&colorMap_[i].color);
		binaryManager_->Register<Vector4>(&colorMap_[i].outlineColor);
	}

	binaryManager_->Write(fileName_);
}

void BlockRender::Load() {
	colorMap_.resize(int(Tetrimino::Count));

	if (!binaryManager_->Boot(fileName_)) {
		//ファイルを開けなかった場合は初期値を入力
		colorMap_[int(Tetrimino::Type::None)] = { ConvertColor(0x00000), ConvertColor(0x00000000) }; //		空白		Air
		colorMap_[int(Tetrimino::Type::S)] = { ConvertColor(0x00ff00ff), ConvertColor(0xffffffff) }; //		赤		S
		colorMap_[int(Tetrimino::Type::Z)] = { ConvertColor(0xff0000ff), ConvertColor(0xffffffff) }; //		緑		Z
		colorMap_[int(Tetrimino::Type::T)] = { ConvertColor(0xff00ffff), ConvertColor(0xffffffff) }; //		紫		T
		colorMap_[int(Tetrimino::Type::O)] = { ConvertColor(0xffff00ff), ConvertColor(0xffffffff) }; //		黄		O
		colorMap_[int(Tetrimino::Type::I)] = { ConvertColor(0x00ffffff), ConvertColor(0xffffffff) }; //		水色		I
		colorMap_[int(Tetrimino::Type::L)] = { ConvertColor(0x0000ffff), ConvertColor(0xffffffff) }; //		青		L
		colorMap_[int(Tetrimino::Type::J)] = { ConvertColor(0xff8000ff), ConvertColor(0xffffffff) }; //		オレンジ	J
		colorMap_[int(Tetrimino::Type::Wall)] = { ConvertColor(0x808080ff), ConvertColor(0x000000ff) }; //	灰色		Wall
		colorMap_[int(Tetrimino::Type::Del)] = { ConvertColor(0xffffffff), ConvertColor(0xffffffff) };//	白		DeleteEffect
		colorMap_[int(Tetrimino::Type::Hold)] = { ConvertColor(0x000000ff), ConvertColor(0xffffffff) };
		colorMap_[int(Tetrimino::Type::Next)] = { ConvertColor(0x000000ff), ConvertColor(0xffffffff) };
		return;
	}

	holdBasePosition_ = binaryManager_->Reverse<Vector3>();
	nextBasePosition_ = binaryManager_->Reverse<Vector3>();
	nextGap_ = binaryManager_->Reverse<Vector3>();

	for (int i = 0; i < int(Tetrimino::Count); ++i) {
		int id = binaryManager_->Reverse<int>();
		Vector4 color = binaryManager_->Reverse<Vector4>();
		Vector4 outlineColor = binaryManager_->Reverse<Vector4>();

		colorMap_[static_cast<int>(i)] = { color, outlineColor };
	}
}
