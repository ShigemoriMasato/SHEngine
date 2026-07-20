#pragma once
#include <Game/Tetris/Field/Field.h>
#include <Game/Tetris/Block/BlockRender.h>
#include <Game/Tetris/Mino/Tetrimino.h>
#include <Game/KeyCoating/KeyCoating.h>
#include <Game/Tetris/Player/Player.h>
#include <Assets/Texture/TextureData.h>

class Tetris {
public:

	Tetris() = default;
	~Tetris() = default;

	void Initialize(KeyCoating* keys, Camera* camera, const Mesh& cubeMesh, SHEngine::TextureData* ddsTexture);
	void Update(float deltaTime);
	void Draw(DCC* cmdObj);
	void DrawImGui();

	bool IsGameOver() const { return gameOver_; }
	int IsLineDeleted() const { return deletedLine_; }

private://定数

	const int fieldWidth_ = 10;
	const int fieldHeight_ = 20;

	std::mt19937 mt;

private: //Debug用

	const float downInterval_ = 1.0f;

private:

	KeyCoating* keys_ = nullptr;

	std::unique_ptr<Field> field_ = nullptr;
	std::unique_ptr<BlockRender> blockRender_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;

	std::unique_ptr<Tetrimino> tetrimino_ = nullptr;

	bool gameOver_ = false;
	int deletedLine_ = 0;

};

