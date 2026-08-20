#include "Tetris.h"

void Tetris::Initialize(KeyCoating* keys, Camera* camera, const Mesh& cubeMesh, SHEngine::TextureData* ddsTexture) {
	field_.Initialize(camera);

	blockRender_.Initialize(fieldWidth_, fieldHeight_, camera, cubeMesh, ddsTexture);

	std::random_device rd;
	mt = std::mt19937(rd());
	tetrimino_.Initialize(mt);

	player_.Initialize(&field_, &tetrimino_);

	scoreManager_.Initialize();

	keys_ = keys;
}

void Tetris::Update(float deltaTime) {
	auto key = keys_->GetKeyStates();
	deletedLine_ = 0;

	float deltaTimeScale = 1.0f + (scoreManager_.GetLevel() - 1) * 0.1f;
	float scaledDeltaTime = deltaTime * deltaTimeScale;

	if (gameOver_ || deltaTime == 0.0f) {
		blockRender_.Update(deltaTime);
		return;
	}

	if (!blockRender_.GetIsEffecting()) {
		player_.SetDownTime(downInterval_);

		bool success = player_.SpawnMino();
		if (!success) {
			gameOver_ = true;
			return;
		}
		gameOver_ = false;

		bool isDroped = player_.Update(scaledDeltaTime, key);

		auto mapData = field_.GetField();
		blockRender_.SetStageData(mapData, player_.GetMoveMino());
		std::vector<std::pair<int, int>> holdPos;
		holdPos = tetrimino_.GetHandle(Tetrimino::Type(player_.GetHoldMino()));
		blockRender_.SetHoldMino(holdPos, Tetrimino::Hold);

		//Next 4個分
		const int nextNum = 4;
		std::vector<std::pair<int, int>> nextPos;
		nextPos.resize(4 * nextNum);
		for (int i = 0; i < nextNum; ++i) {
			auto nextType = tetrimino_.GetNextTetrimino(i);
			auto next = tetrimino_.GetHandle(nextType);
			for(int j = 0; j < next.size(); ++j) {
				nextPos[j + (4 * i)] = next[j];
			}
		}
		blockRender_.SetNextMino(nextPos, int(Tetrimino::Type::Next));

		fillLines_ = field_.FillLineIndex();
		if (!fillLines_.empty()) {
			field_.DeleteFillLine();
			blockRender_.BeginDeleteEffect(fillLines_, field_.GetField());
			deletedLine_ = int(fillLines_.size());
			scoreManager_.DeleteLine(deletedLine_, player_.GetIsTSpin(), field_.IsAllClear());
		} else if (isDroped) {
			scoreManager_.ResetLen();
		}

	}

	blockRender_.Update(deltaTime);
}

void Tetris::Draw(DCC* cmdObj) {
	blockRender_.Draw(cmdObj);
}

void Tetris::DrawImGui() {
	blockRender_.DrawImGui();
}

std::vector<Transform> Tetris::DeleteLinesTransform() const {
	return blockRender_.DeleteLinesTransform();
}
